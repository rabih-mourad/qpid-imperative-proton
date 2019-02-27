#include <proton/imperative/Connection.hpp>
#include <proton/imperative/Session.hpp>

#include <iostream>

using namespace proton;

Connection::Connection(container& cont)
   :m_container(cont)
{}

Connection::Connection(Connection&& c)
   : m_connectionHandler(std::move(c.m_connectionHandler)),
   m_container(c.m_container)
{}

Connection::~Connection()
{
   close();
}

void Connection::close()
{
   if (m_connectionHandler.m_manager.canBeClosed())
   {
      auto f = m_connectionHandler.m_manager.close();
      m_connectionHandler.m_connection.work_queue().add([=]() {m_connectionHandler.m_connection.close(); });
      f.get();
   }
}

Session Connection::createSession()
{
   m_connectionHandler.m_manager.checkForExceptions();
   return Session(m_connectionHandler.m_connection);
}

std::future<void> Connection::open(const std::string& url, connection_options conn_opts)
{
   if (!m_connectionHandler.m_manager.canBeOpened())
   {
      throw std::runtime_error("Connection is already opened. Can't be opened it multiple times");
   }
   auto f = m_connectionHandler.m_manager.open();
   m_container.connect(url, conn_opts.handler(m_connectionHandler));
   return f;
}

// ConnectionHandler

Connection::ConnectionHandler::ConnectionHandler(ConnectionHandler&& c)
   : m_connection(std::move(c.m_connection)),
   m_manager(std::move(c.m_manager))
{}

void Connection::ConnectionHandler::on_connection_open(connection& conn)
{
   m_connection = conn;
   std::cout << "client on_connection_open" << std::endl;
   m_manager.finishedOpenning();
}

void Connection::ConnectionHandler::on_connection_close(connection&)
{
   std::cout << "client on_connection_close" << std::endl;
   m_manager.finishedClosing();
}

void Connection::ConnectionHandler::on_connection_error(connection& conn)
{
   std::cout << "client on_connection_error" << std::endl;
   m_manager.handlePnError(conn.error().what());
}

void Connection::ConnectionHandler::on_transport_open(transport&)
{
   std::cout << "client on_transport_open" << std::endl;
}

void Connection::ConnectionHandler::on_transport_close(transport&)
{
   std::cout << "client on_transport_close" << std::endl;
}

void Connection::ConnectionHandler::on_transport_error(transport& t)
{
   std::cout << "client on_transport_error" << std::endl;
   m_manager.handlePnError(t.error().what());
}
