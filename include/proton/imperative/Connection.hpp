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
#include <proton/imperative/CloseRegistry.hpp>

namespace proton {

class PROTON_IMPERATIVE_API Connection
{
public:
   void close();
   Session openSession(session_options sess_opts);

   ~Connection();
   Connection(Connection&& c) = default;
   Connection(const Connection& other) = delete;
   Connection& operator=(const Connection& other) = delete;
   Connection& operator=(Connection&& other) = delete;

private:
   struct PROTON_IMPERATIVE_API ConnectionHandler : public messaging_handler
   {
      ConnectionHandler(CloseRegistry*);
      ConnectionHandler(ConnectionHandler&& c) = default;
      ConnectionHandler(const ConnectionHandler& other) = delete;
      ConnectionHandler& operator=(const ConnectionHandler& other) = delete;
      ConnectionHandler& operator=(ConnectionHandler&& other) = delete;

      void on_connection_open(connection&) override;
      void on_connection_close(connection&) override;
      void on_connection_error(connection&) override;
      void on_transport_open(transport&) override;
      void on_transport_close(transport&) override;
      void on_transport_error(transport&) override;
      void releasePnMemberObjects(const std::string&);

      connection m_connection;
      work_queue* m_work;
      PnObjectLifeManager m_manager;
   };

   Connection(container& c, const std::string& url, connection_options conn_opts, CloseRegistry* containerCloseRegistry);
   // Should be public but because the connection creation must be always synchronous 
   // in order that the work queue will not be correctly set
   std::future<void> getOpenFuture();
   friend class Container;

   std::unique_ptr<ConnectionHandler> m_connectionHandler;
};

}

#endif
