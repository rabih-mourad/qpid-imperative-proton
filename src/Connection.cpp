#include <proton/imperative/Connection.hpp>
#include <proton/imperative/Session.hpp>

#include <iostream>

using namespace proton;


Connection::Connection(container& c, const std::string& url, connection_options conn_opts)
   : m_connectionHandler(new ConnectionHandler)
{
   auto f = m_connectionHandler->m_manager.getOpenFuture();
   c.connect(url, conn_opts.handler(*m_connectionHandler));
   f.get();
}

Connection::Connection(Connection&& c)
   : m_connectionHandler(std::move(c.m_connectionHandler))
{}

Connection::~Connection()
{
   close();
}

void Connection::close()
{
   if (m_connectionHandler && !m_connectionHandler->m_manager.hasBeenClosedOrInError())
   {
      //we get the future before launching the action because std promise is not thread safe between get and set
      auto f = m_connectionHandler->m_manager.close();
      m_connectionHandler->m_work->add([=]() {m_connectionHandler->m_connection.close(); });
      //we need to wait before destroying because the handler must live till the end
      f.get();
   }
}

Session Connection::openSession(session_options sess_opts)
{
   m_connectionHandler->m_manager.checkForExceptions();
   return Session(m_connectionHandler->m_connection, m_connectionHandler->m_work, sess_opts);
}

//std::future<void> Connection::getOpenFuture()
//{
//   return std::move(m_connectionHandler->m_manager.getOpenFuture());
//}

// ConnectionHandler

Connection::ConnectionHandler::ConnectionHandler(ConnectionHandler&& c)
   : m_manager(std::move(c.m_manager)),
   m_connection(std::move(c.m_connection))
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
   std::cout << "client on_connection_close" << std::endl;
}

void Connection::ConnectionHandler::on_connection_error(connection& conn)
{
   std::cout << "client on_connection_error: " << conn.error().what() << std::endl;
   m_manager.handlePnError(conn.error().what());
   conn.close(conn.error());
}

void Connection::ConnectionHandler::on_transport_open(transport&)
{
   std::cout << "client on_transport_open" << std::endl;
}

void Connection::ConnectionHandler::on_transport_close(transport&)
{
   std::cout << "client on_transport_close" << std::endl;
   m_manager.handlePnClose(std::bind(&Connection::ConnectionHandler::releasePnMemberObjects, this));
}

void Connection::ConnectionHandler::on_transport_error(transport& t)
{
   std::cout << "client on_transport_error: " << t.error().what() << std::endl;
   m_manager.handlePnError(t.error().what());
}

void Connection::ConnectionHandler::releasePnMemberObjects()
{
   // Reseting pn objects for thread safety
   m_connection = connection();
}