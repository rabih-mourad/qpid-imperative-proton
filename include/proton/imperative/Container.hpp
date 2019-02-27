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
   //Synchronous methods
   Container();
   ~Container();
   void close();

   Connection createConnection();

private:
   struct ContainerHandler : public messaging_handler
   {
      void on_container_start(proton::container&) override;
      void on_container_stop(container&) override;

      PnObjectLifeManager m_manager;
   };

private:
   ContainerHandler m_containerHandler;
   container m_container;
   ThreadRAII m_thread;
};

}

#endif
