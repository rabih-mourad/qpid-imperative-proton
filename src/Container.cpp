#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>

#include <iostream>

using namespace proton;

Container::Container()
   :m_container(m_containerHandler)
{
   auto f = m_containerHandler.m_manager.getOpenFuture();
   m_thread = ThreadRAII(std::thread([&]() {
      try {
         m_container.auto_stop(false);
         m_container.run();
      } catch (const std::exception& e) {
         std::cout << "std::exception caught, message:" << e.what() << std::endl;
      }}));
   f.get();
}

Container::~Container()
{
   close();
}

void Container::close()
{
   if (!m_containerHandler.m_manager.hasBeenClosedOrInError())
   {
      // we get the future before launching the action because std promise is not thread safe between get and set
      auto f = m_containerHandler.m_manager.close();
      m_container.stop();
      f.get();
   }
}

Connection Container::openConnection(const std::string& url, connection_options conn_opts)
{
   m_containerHandler.m_manager.checkForExceptions();
   return Connection(m_container, url, conn_opts);
}

//ContainerHandler

void Container::ContainerHandler::on_container_start(proton::container&)
{
   std::cout << "client on_container_start" << std::endl;
   m_manager.handlePnOpen();
}

void Container::ContainerHandler::on_container_stop(container&)
{
   std::cout << "client on_container_stop" << std::endl;
   m_manager.handlePnClose([]{});
}
