#include <proton/imperative/Session.hpp>
#include <proton/imperative/Sender.hpp>

#include <iostream>

using namespace proton;

Session::Session(connection conn)
   :m_connection(conn)
{}

Session::Session(Session&& s)
   : m_sessionHandler(std::move(s.m_sessionHandler)),
   m_connection(s.m_connection)
{}

Session::~Session()
{
   close();
}

void Session::close()
{
   if (m_sessionHandler.m_manager.canBeClosed())
   {
      auto f = m_sessionHandler.m_manager.close();
      m_sessionHandler.m_session.work_queue().add([=]() {m_sessionHandler.m_session.close(); });
      f.get();
   }
}

Sender Session::createSender()
{
   m_sessionHandler.m_manager.checkForExceptions();
   return Sender(m_sessionHandler.m_session);
}

Receiver Session::createReceiver()
{
   m_sessionHandler.m_manager.checkForExceptions();
   return Receiver(m_sessionHandler.m_session);
}

std::future<void> Session::open(session_options sess_opts)
{
   if (!m_sessionHandler.m_manager.canBeOpened())
   {
      throw std::runtime_error("Session is already opened. Can't be opened it multiple times");
   }

   auto f = m_sessionHandler.m_manager.open();
   sess_opts.handler(m_sessionHandler);
   m_connection.work_queue().add([=]() {m_connection.open_session(sess_opts); });
   return f;
}

// SessionHandler

Session::SessionHandler::SessionHandler(SessionHandler&& s)
   : m_session(std::move(s.m_session)),
   m_manager(std::move(s.m_manager))
{}

void Session::SessionHandler::on_session_open(session& sess)
{
   m_session = sess;
   std::cout << "client on_session_open" << std::endl;
   m_manager.finishedOpenning();
}

void Session::SessionHandler::on_session_close(session&)
{
   std::cout << "client on_session_close" << std::endl;
   m_manager.finishedClosing();
}

void Session::SessionHandler::on_session_error(session& sess)
{
   std::cout << "client on_session_error" << std::endl;
   m_manager.handlePnError(sess.error().what());
}

