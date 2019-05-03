#include <proton/imperative/Receiver.hpp>
#include <proton/imperative/Delivery.hpp>

#include <iostream>

using namespace proton;

Receiver::Receiver(work_queue* work, session& sess, const std::string& address, receiver_options rec_opts)
   : m_receiverHandler(new ReceiverHandler(work))
{
   rec_opts.handler(*m_receiverHandler).credit_window(0);
   // TO DO see if we can use [=] instead of [&]
   receiver& rec = m_receiverHandler->m_receiver;
   work->add([=, &sess, &rec]() { rec = sess.open_receiver(address, rec_opts); });
}

Receiver::Receiver(Receiver&& r)
   : m_receiverHandler(std::move(r.m_receiverHandler))
{}

Receiver::~Receiver()
{
   close();
}

void Receiver::close()
{
   if (m_receiverHandler && !m_receiverHandler->m_manager.hasBeenClosedOrInError())
   {
      //we get the future before launching the action because std promise is not thread safe between get and set
      auto f = m_receiverHandler->m_manager.close();
      m_receiverHandler->m_work->add([=]() {m_receiverHandler->m_receiver.close(); });
      f.get();
   }
}

std::future<Delivery> Receiver::receive()
{
   m_receiverHandler->m_manager.checkForExceptions();
   return m_receiverHandler->receive();
}

std::future<void> Receiver::getOpenFuture()
{
   return m_receiverHandler->m_manager.getOpenFuture();
}

// ReceiverHandler

Receiver::ReceiverHandler::ReceiverHandler(work_queue* work)
   : m_work(work)
{}

Receiver::ReceiverHandler::ReceiverHandler(ReceiverHandler&& r)
   : m_manager(std::move(r.m_manager)),
   m_receiver(std::move(r.m_receiver)),
   m_work(r.m_work),
   m_promisesQueue(std::move(r.m_promisesQueue))
{}

std::future<Delivery> Receiver::ReceiverHandler::receive()
{
   std::promise<Delivery> msgPromise;
   std::future<Delivery> msgFuture = msgPromise.get_future();
   m_work->add([=]() { m_receiver.add_credit(1); });
   std::lock_guard<std::mutex> l(m_queueLock);
   m_promisesQueue.push(std::move(msgPromise));
   return msgFuture;
}

void Receiver::ReceiverHandler::on_receiver_open(receiver& rec)
{
   std::cout << "client on_receiver_open" << std::endl;
   m_manager.handlePnOpen();
}

void Receiver::ReceiverHandler::on_receiver_close(receiver&)
{
   std::cout << "client on_receiver_close" << std::endl;
   m_manager.handlePnClose(std::bind(&Receiver::ReceiverHandler::releasePnMemberObjects, this));
}

void Receiver::ReceiverHandler::on_receiver_error(receiver& rec)
{
   std::cout << "client on_receiver_error" << std::endl;
   m_manager.handlePnError(rec.error().what());

   std::lock_guard<std::mutex> l(m_queueLock);
   while (!m_promisesQueue.empty())
   {
      m_promisesQueue.front().set_exception(std::make_exception_ptr(std::runtime_error(rec.error().what())));
      m_promisesQueue.pop();
   }
}

void Receiver::ReceiverHandler::on_message(delivery& d, message& m)
{
   std::cout << "client on_message" << std::endl;
   {
      std::lock_guard<std::mutex> l1(m_listLock);
      m_unclosedDeliveries.emplace_back(d);
   }
   Delivery dmsg(m, m_work, &(m_unclosedDeliveries.back()), &m_unclosedDeliveries, &m_listLock);
   {
      std::lock_guard<std::mutex> l2(m_queueLock);
      m_promisesQueue.front().set_value(dmsg);
      m_promisesQueue.pop();
   }
}

void Receiver::ReceiverHandler::releasePnMemberObjects()
{
   // Reseting pn objects for thread safety
   std::lock_guard<std::mutex> l(m_listLock);
   m_unclosedDeliveries.clear();
   m_receiver = receiver();

}
