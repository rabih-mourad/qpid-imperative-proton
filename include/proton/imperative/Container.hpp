#ifndef PROTON_IMPERATIVE_CONTAINER_HPP
#define PROTON_IMPERATIVE_CONTAINER_HPP

#include <proton/imperative/config.hpp>
#include <proton/imperative/ThreadRAII.hpp>
#include <proton/imperative/PnObjectLifeManager.hpp>

#include <proton/connection_options.hpp>
#include <proton/container.hpp>
#include <proton/messaging_handler.hpp>

#include <string>
#include <future>

namespace proton {

class Connection;

class PROTON_IMPERATIVE_API Container
{
public:
   Container();
   ~Container();
   Container(const Container& other) = delete;
   Container& operator=(const Container& other) = delete;
   Container& operator=(Container&& other) = delete;
   Container(Container&& c) = delete;

   void close();
   //This method is synchronous because there is no thread safe way in proton to get the connection
   Connection openConnection(const std::string& url, connection_options conn_opts);

private:
   struct ContainerHandler : public messaging_handler
   {
      ContainerHandler();
      ContainerHandler(const ContainerHandler& other) = delete;
      ContainerHandler& operator=(const ContainerHandler& other) = delete;
      ContainerHandler& operator=(ContainerHandler&& other) = delete;
      ContainerHandler(ContainerHandler&& c) = delete;

      void on_container_start(proton::container&) override;
      void on_container_stop(container&) override;

      PnObjectLifeManager m_manager;
   };

   ContainerHandler m_containerHandler;
   container m_container;
   error_condition m_pnErr;
   ThreadRAII m_thread;
};
}

#endif
