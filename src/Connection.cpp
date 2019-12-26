#include <proton/imperative/Connection.hpp>
#include <proton/imperative/Session.hpp>

#include <iostream>

using namespace proton;

static std::string containerCloseMsg("Container explicitly closed");

Connection::Connection(container& c, const std::string& url, connection_options conn_opts, CloseRegistry* containerCloseRegistry)
   : m_connectionHandler(new ConnectionHandler(containerCloseRegistry))
{
   c.connect(url, conn_opts.handler(*m_connectionHandler));
}

Connection::~Connection()
{
   close();
}

void Connection::close()
{
   if (m_connectionHandler == nullptr)
   {
      // we pass here when the move constructor called
      return;
   }

   std::unique_lock<std::mutex> lock(m_connectionHandler->m_manager.m_mutex);
   if (!m_connectionHandler->m_manager.hasBeenClosed())
   {
      std::function<void()> closeFn = [=]() { m_connectionHandler->m_connection.close(); };
      //we get the future before launching the action because std promise is not thread safe between get and set
      auto f = m_connectionHandler->m_manager.close();
      if (!m_connectionHandler->m_manager.isInError())
      {
         m_connectionHandler->m_work->add(closeFn);
      }
      lock.unlock();
      //we need to wait before destroying because the handler must live till the end
      f.wait();
   }
}

Session Connection::openSession(session_options sess_opts)
{
   std::lock_guard<std::mutex> lock(m_connectionHandler->m_manager.m_mutex);
   m_connectionHandler->m_manager.checkForExceptions();
   return Session(m_connectionHandler->m_connection, m_connectionHandler->m_work, sess_opts, m_connectionHandler->m_manager.getCloseRegistry());
}

std::future<void> Connection::getOpenFuture()
{
   return std::move(m_connectionHandler->m_manager.getOpenFuture());
}

// ConnectionHandler

Connection::ConnectionHandler::ConnectionHandler(CloseRegistry* containerCloseHandler)
   :m_manager(containerCloseHandler, [&](const std::string& str) { releasePnMemberObjects(str); })
{}

void Connection::ConnectionHandler::on_connection_open(connection& conn)
{
   std::cout << "client on_connection_open" << std::endl;
   m_connection = conn;
   m_work = &m_connection.work_queue();
   m_manager.handlePnOpen();
}

void Connection::ConnectionHandler::on_connection_close(connection&)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   std::cout << "client on_connection_close" << std::endl;
}

void Connection::ConnectionHandler::on_connection_error(connection& conn)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   std::cout << "client on_connection_error: " << conn.error().what() << std::endl;
   m_manager.handlePnError(conn.error().what());
}

void Connection::ConnectionHandler::on_transport_open(transport&)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   std::cout << "client on_transport_open" << std::endl;
}

void Connection::ConnectionHandler::on_transport_close(transport&)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   std::cout << "client on_transport_close" << std::endl;
   m_manager.handlePnClose();
}

void Connection::ConnectionHandler::on_transport_error(transport& t)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   std::cout << "client on_transport_error: " << t.error().what() << std::endl;

   if (t.error().description().find(containerCloseMsg) != std::string::npos)
   {
      m_manager.handlePnError(m_manager.errorParentClosed);
      m_manager.handlePnClose();
      return;
   }

   m_manager.handlePnError(t.error().what());
}

void Connection::ConnectionHandler::releasePnMemberObjects(const std::string&)
{
   // Reseting pn objects for thread safety
   std::cout << "---Connection releasePnMemberObjects before" << std::endl;
   m_connection = connection();
   std::cout << "---Connection releasePnMemberObjects" << std::endl;
}
