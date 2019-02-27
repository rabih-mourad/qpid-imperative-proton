#include <proton/imperative/Receiver.hpp>

#include <iostream>

using namespace proton;

Receiver::Receiver(session sess)
   :m_session(sess)
{}

Receiver::Receiver(Receiver&& r)
   : m_receiverHandler(std::move(r.m_receiverHandler)), m_session(r.m_session)
{}

Receiver::~Receiver()
{
   close();
}

void Receiver::close()
{
   if (m_receiverHandler.m_manager.canBeClosed())
   {
      auto f = m_receiverHandler.m_manager.close();
      m_receiverHandler.m_receiver.work_queue().add([=]() {m_receiverHandler.m_receiver.close(); });
      f.get();
   }
}

std::future<Receiver::Delivery> Receiver::receive()
{
   return m_receiverHandler.receive();
}

std::future<void> Receiver::open(const std::string& address, receiver_options rec_opts)
{
   if (!m_receiverHandler.m_manager.canBeOpened())
   {
      throw std::runtime_error("Connection is already opened. Can't be opened it multiple times");
   }

   auto f = m_receiverHandler.m_manager.open();
   rec_opts.handler(m_receiverHandler).credit_window(0);
   m_session.work_queue().add([=]() {m_session.open_receiver(address, rec_opts); });
   return f;
}

// ReceiverHandler

Receiver::ReceiverHandler::ReceiverHandler(ReceiverHandler&& r)
   : m_receiver(std::move(r.m_receiver)),
   m_manager(std::move(r.m_manager))
{}

std::future<Receiver::Delivery> Receiver::ReceiverHandler::receive()
{
   std::promise<Delivery> msgPromise;
   std::future<Delivery> msgFuture = msgPromise.get_future();
   m_receiver.work_queue().add([=]() {m_receiver.add_credit(1); });
   m_promisesQueue.push(std::move(msgPromise));
   return msgFuture;
}

void Receiver::ReceiverHandler::on_receiver_open(receiver& rec)
{
   m_receiver = rec;
   std::cout << "client on_receiver_open" << std::endl;
   m_manager.finishedOpenning();
}

void Receiver::ReceiverHandler::on_receiver_close(receiver&)
{
   std::cout << "client on_receiver_close" << std::endl;
   m_manager.finishedClosing();
}

void Receiver::ReceiverHandler::on_receiver_error(receiver& rec)
{
   std::cout << "client on_receiver_error" << std::endl;
   m_manager.handlePnError(rec.error().what());
}

void Receiver::ReceiverHandler::on_message(delivery& d, message& m)
{
   std::cout << "client on_message" << std::endl;
   Receiver::Delivery dmsg(d, m);
   m_promisesQueue.front().set_value(dmsg);
   m_promisesQueue.pop();
}
