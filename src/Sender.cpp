#include <proton/imperative/Sender.hpp>

#include <iostream>

using namespace proton;

Sender::Sender(session sess)
   :m_session(sess)
{}

Sender::Sender(Sender&& s)
   : m_senderHandler(std::move(s.m_senderHandler)),
   m_session(s.m_session)
{}

Sender::~Sender()
{
   close();
}

void Sender::close()
{
   if (m_senderHandler.m_manager.canBeClosed())
   {
      auto f = m_senderHandler.m_manager.close();
      m_senderHandler.m_sender.work_queue().add([=]() {m_senderHandler.m_sender.close(); });
      f.get();
   }
}

std::future<void> Sender::send(const message& mess)
{
   return m_senderHandler.send(mess);
}

std::future<void> Sender::open(const std::string& address, sender_options send_opts)
{
   if (!m_senderHandler.m_manager.canBeOpened())
   {
      throw std::runtime_error("Sender is already opened. Can't be opened it multiple times");
   }
   auto f = m_senderHandler.m_manager.open();
   send_opts.handler(m_senderHandler);
   m_session.work_queue().add([=]() {m_session.open_sender(address, send_opts); });
   return f;
}

// SenderHandler

Sender::SenderHandler::SenderHandler(SenderHandler&& s)
   : m_sender(std::move(s.m_sender)),
   m_manager(std::move(s.m_manager))
{}

std::future<void> Sender::SenderHandler::send(const message& mess)
{
   std::promise<void> ackPromise;
   std::future<void> ackFuture = ackPromise.get_future();
   m_sender.work_queue().add([=, &ackPromise]() {
      tracker track = m_sender.send(mess);
      m_sentRequests.emplace(track, std::move(ackPromise));
   });
   return ackFuture;
}

void Sender::SenderHandler::on_sender_open(sender& sen)
{
   m_sender = sen;
   std::cout << "client on_sender_open" << std::endl;
   m_manager.finishedOpenning();
}

void Sender::SenderHandler::on_sender_close(sender&)
{
   std::cout << "client on_sender_close" << std::endl;
   m_manager.finishedClosing();
}

void Sender::SenderHandler::on_sender_error(sender& sen)
{
   std::cout << "client on_sender_error" << std::endl;
   m_manager.handlePnError(sen.error().what());
}

void Sender::SenderHandler::on_tracker_accept(tracker& track)
{
   std::cout << "client on_tracker_accept" << std::endl;
   std::promise<void> promise = removeTrackerFromMapIfFoundElseThow(track);
   promise.set_value();
}

void Sender::SenderHandler::on_tracker_reject(tracker& track)
{
   std::cout << "client on_tracker_settle" << std::endl;
   std::promise<void> promise = removeTrackerFromMapIfFoundElseThow(track);
   auto except = std::make_exception_ptr(std::runtime_error("message was rejected by peer"));
   promise.set_exception(except);
}

void Sender::SenderHandler::on_tracker_release(tracker& track)
{
   std::cout << "client on_tracker_settle" << std::endl;
   std::promise<void> promise = removeTrackerFromMapIfFoundElseThow(track);
   auto except = std::make_exception_ptr(std::runtime_error("message was released by peer"));
   promise.set_exception(except);
}

std::promise<void> Sender::SenderHandler::removeTrackerFromMapIfFoundElseThow(tracker& track)
{
   std::map<tracker, std::promise<void>>::iterator it = m_sentRequests.find(track);
   if (it == m_sentRequests.end())
   {
      std::string err("Illegal state tracker was not found: received ack for a message not sent");
      m_manager.handlePnError(err);
      throw std::runtime_error(err);
   }
   std::promise<void> promise = std::move(it->second);
   m_sentRequests.erase(it);
   return std::move(promise);
}