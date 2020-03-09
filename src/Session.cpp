#include <proton/imperative/Session.hpp>
#include <proton/imperative/Sender.hpp>
#include <proton/imperative/Receiver.hpp>

#include <Logger.hpp>

using namespace proton;

namespace {
   Log debug_log;
}

Session::Session(connection& con, work_queue* work, session_options sess_opts, CloseRegistry* connectionCloseRegistry)
   : m_sessionHandler(new SessionHandler(work, connectionCloseRegistry))
{
   sess_opts.handler(*m_sessionHandler);
   // TO DO see if we can use [=] instead of [&]
   session& sess = m_sessionHandler->m_session;
   m_sessionHandler->m_work->add([=, &con, &sess]() {
      sess = con.open_session(sess_opts); 
   });
}

Session::~Session()
{
   close();
}

void Session::close()
{
   if (m_sessionHandler == nullptr)
   {
      // we pass here when the move constructor called
      return;
   }

   std::unique_lock<std::mutex> lock(m_sessionHandler->m_manager.m_mutex);
   if (!m_sessionHandler->m_manager.hasBeenClosed())
   {
      std::function<void()> closeFn = [=]() { m_sessionHandler->m_session.close(); };
      //we get the future before launching the action because std promise is not thread safe between get and set
      auto f = m_sessionHandler->m_manager.close();
      if (!m_sessionHandler->m_manager.isInError())
      {
         m_sessionHandler->m_work->add(closeFn);
      }
      lock.unlock();
      f.wait();
   }
}

Sender Session::openSender(const std::string& address, sender_options send_opts)
{
   std::lock_guard<std::mutex> lock(m_sessionHandler->m_manager.m_mutex);
   m_sessionHandler->m_manager.checkForExceptions();
   return Sender(m_sessionHandler->m_work, m_sessionHandler->m_session, address, send_opts, m_sessionHandler->m_manager.getCloseRegistry());
}

Receiver Session::openReceiver(const std::string& address, receiver_options rec_opts)
{
   std::lock_guard<std::mutex> lock(m_sessionHandler->m_manager.m_mutex);
   m_sessionHandler->m_manager.checkForExceptions();
   return Receiver(m_sessionHandler->m_work, m_sessionHandler->m_session, address, rec_opts, m_sessionHandler->m_manager.getCloseRegistry());
}

std::future<void> Session::getOpenFuture()
{
   return m_sessionHandler->m_manager.getOpenFuture();
}

// SessionHandler

Session::SessionHandler::SessionHandler(work_queue* w, CloseRegistry* connectionCloseHandler)
   :m_work(w), m_manager(connectionCloseHandler, [&](const std::string& str) {releasePnMemberObjects(str); })
{}

void Session::SessionHandler::on_session_open(session& sess)
{
   debug_log << "client on_session_open" << std::endl;
   m_manager.handlePnOpen();
}

void Session::SessionHandler::on_session_close(session&)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   debug_log << "client on_session_close" << std::endl;
   m_manager.handlePnClose();
}

void Session::SessionHandler::on_session_error(session& sess)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   debug_log << "client on_session_error" << std::endl;
   m_manager.handlePnError(sess.error().what());
}

void Session::SessionHandler::releasePnMemberObjects(const std::string&)
{
   // Reseting pn objects for thread safety
   m_session = session();
}
