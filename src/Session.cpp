#include <proton/imperative/Session.hpp>
#include <proton/imperative/Sender.hpp>
#include <proton/imperative/Receiver.hpp>

#include <iostream>

using namespace proton;

Session::Session(connection& con, work_queue* work, session_options sess_opts)
   : m_sessionHandler(new SessionHandler(work))
{
   sess_opts.handler(*m_sessionHandler);
   // TO DO see if we can use [=] instead of [&]
   session& sess = m_sessionHandler->m_session;
   m_sessionHandler->m_work->add([=, &con, &sess]() {
      sess = con.open_session(sess_opts); 
   });
}

Session::Session(Session&& s)
   : m_sessionHandler(std::move(s.m_sessionHandler))
{}

Session::~Session()
{
   close();
}

void Session::close()
{
   if (m_sessionHandler && !m_sessionHandler->m_manager.hasBeenClosedOrInError())
   {
      //we get the future before launching the action because std promise is not thread safe between get and set
      auto f = m_sessionHandler->m_manager.close();
      m_sessionHandler->m_work->add([=]() {m_sessionHandler->m_session.close(); });
      f.get();
   }
}

Sender Session::openSender(const std::string& address, sender_options send_opts)
{
   m_sessionHandler->m_manager.checkForExceptions();
   return Sender(m_sessionHandler->m_work, m_sessionHandler->m_session, address, send_opts);
}

Receiver Session::openReceiver(const std::string& address, receiver_options rec_opts)
{
   m_sessionHandler->m_manager.checkForExceptions();
   return Receiver(m_sessionHandler->m_work, m_sessionHandler->m_session, address, rec_opts);
}

std::future<void> Session::getOpenFuture()
{
   return m_sessionHandler->m_manager.getOpenFuture();
}

// SessionHandler

Session::SessionHandler::SessionHandler(work_queue* w)
   :m_work(w)
{}

Session::SessionHandler::SessionHandler(SessionHandler&& s)
   : m_manager(std::move(s.m_manager)),
   m_session(std::move(s.m_session))
{}

void Session::SessionHandler::on_session_open(session& sess)
{
   std::cout << "client on_session_open" << std::endl;
   m_manager.handlePnOpen();
}

void Session::SessionHandler::on_session_close(session&)
{
   std::cout << "client on_session_close" << std::endl;
   m_manager.handlePnClose(std::bind(&Session::SessionHandler::releasePnMemberObjects, this));
}

void Session::SessionHandler::on_session_error(session& sess)
{
   std::cout << "client on_session_error" << std::endl;
   m_manager.handlePnError(sess.error().what());
}

void Session::SessionHandler::releasePnMemberObjects()
{
   // Reseting pn objects for thread safety
   m_session = session();
}
