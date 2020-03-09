#include <proton/imperative/Receiver.hpp>
#include <proton/imperative/Delivery.hpp>

#include <Logger.hpp>

using namespace proton;

namespace {
   Log debug_log;
}

Receiver::Receiver(work_queue* work, session& sess, const std::string& address, receiver_options rec_opts, CloseRegistry* sessionCloseRegistry)
   : m_receiverHandler(new ReceiverHandler(work, sessionCloseRegistry))
{
   rec_opts.handler(*m_receiverHandler).credit_window(0);
   // TO DO see if we can use [=] instead of [&]
   receiver& rec = m_receiverHandler->m_receiver;
   work->add([=, &sess, &rec]() { rec = sess.open_receiver(address, rec_opts); });
}

Receiver::~Receiver()
{
   close();
}

void Receiver::close()
{
   if (m_receiverHandler == nullptr)
   {
      // we pass here when the move constructor called
      return;
   }

   std::unique_lock<std::mutex> lock(m_receiverHandler->m_manager.m_mutex);
   if (!m_receiverHandler->m_manager.hasBeenClosed())
   {
      std::function<void()> closeFn = [=]() { m_receiverHandler->m_receiver.close(); };
      //we get the future before launching the action because std promise is not thread safe between get and set
      auto f = m_receiverHandler->m_manager.close();
      if (!m_receiverHandler->m_manager.isInError())
      {
         m_receiverHandler->m_work->add(closeFn);
      }
      lock.unlock();
      f.wait();
   }
}

std::future<Delivery> Receiver::receive()
{
   std::lock_guard<std::mutex> lock(m_receiverHandler->m_manager.m_mutex);
   m_receiverHandler->m_manager.checkForExceptions();
   return m_receiverHandler->receive();
}

std::future<void> Receiver::getOpenFuture()
{
   return m_receiverHandler->m_manager.getOpenFuture();
}

// ReceiverHandler

Receiver::ReceiverHandler::ReceiverHandler(work_queue* work, CloseRegistry* sessionCloseRegistry)
   : m_work(work), m_manager(sessionCloseRegistry, [&](const std::string& str) {releasePnMemberObjects(str); })
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
   debug_log << "client on_receiver_open" << std::endl;
   m_manager.handlePnOpen();
}

void Receiver::ReceiverHandler::on_receiver_close(receiver&)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   debug_log << "client on_receiver_close" << std::endl;
   debug_log << "pointer : " << &m_unclosedDeliveries << "  size" << m_unclosedDeliveries.size() << std::endl;
   m_manager.handlePnClose();
}

void Receiver::ReceiverHandler::on_receiver_error(receiver& rec)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   debug_log << "client on_receiver_error" << std::endl;
   m_manager.handlePnError(rec.error().what());
}

void Receiver::ReceiverHandler::on_message(delivery& d, message& m)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   debug_log << "client on_message" << std::endl;
   {
      std::lock_guard<std::mutex> l1(m_listLock);
      m_unclosedDeliveries.emplace_back(d);
   }
   {
      Delivery dmsg(m, m_work, &(m_unclosedDeliveries.back()), &m_unclosedDeliveries, &m_listLock, m_manager.getCloseRegistry());
      std::lock_guard<std::mutex> l2(m_queueLock);
      m_promisesQueue.front().set_value(std::move(dmsg));
      m_promisesQueue.pop();
   }
}

void Receiver::ReceiverHandler::releasePnMemberObjects(const std::string& str)
{
   // Reseting pn objects for thread safety
   std::lock_guard<std::mutex> l(m_listLock);
   m_unclosedDeliveries.clear();
   m_receiver = receiver();

   std::lock_guard<std::mutex> lock(m_queueLock);
   while (!m_promisesQueue.empty())
   {
      m_promisesQueue.front().set_exception(std::make_exception_ptr(std::runtime_error(str)));
      m_promisesQueue.pop();
   }
}
