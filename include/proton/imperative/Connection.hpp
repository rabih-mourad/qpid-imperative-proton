#ifndef PROTON_IMPERATIVE_CONNECTION_HPP
#define PROTON_IMPERATIVE_CONNECTION_HPP

#include <proton/work_queue.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/connection.hpp>
#include <proton/transport.hpp>

#include <proton/imperative/config.hpp>
#include <proton/imperative/Container.hpp>
#include <proton/imperative/Session.hpp>
#include <proton/imperative/PromiseWithActiveFlag.hpp>
#include <proton/imperative/PnObjectLifeManager.hpp>

namespace proton {

class PROTON_IMPERATIVE_API Connection
{
private:
   struct ConnectionHandler : public messaging_handler
   {
      ConnectionHandler() = default;
      ConnectionHandler(ConnectionHandler&& c);
      void on_connection_open(connection&) override;
      void on_connection_close(connection&) override;
      void on_connection_error(connection&) override;
      void on_transport_open(transport&) override;
      void on_transport_close(transport&) override;
      void on_transport_error(transport&) override;

      PnObjectLifeManager m_manager;
      connection m_connection;
   };

public:
   std::future<void> open(const std::string& url, connection_options conn_opts);
   void close();
   Session createSession();

   Connection(container& cont);
   Connection(Connection&& c);
   ~Connection();

private:
   ConnectionHandler m_connectionHandler;
   //by reference because if we copy the container it crashes
   container& m_container;
};

}

#endif
