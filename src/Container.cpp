#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>

#include <iostream>

using namespace proton;

Container::Container()
   :m_container(m_containerHandler)
{
   m_thread = ThreadRAII(std::thread([&]() {
      try {
         m_container.auto_stop(false);
         m_container.run();
      } catch (const std::exception& e) {
         std::cout << "std::exception caught, message:" << e.what() << std::endl;
      }}));
   m_containerHandler.m_manager.open().get();
}

Container::~Container()
{
   close();
}

void Container::close()
{
   if (m_containerHandler.m_manager.canBeClosed())
   {
      m_container.stop();
      m_containerHandler.m_manager.close().get();
   }
}

Connection Container::createConnection()
{
   return Connection(m_container);
}

//ContainerHandler

void Container::ContainerHandler::on_container_start(proton::container&)
{
   std::cout << "client on_container_start" << std::endl;
   m_manager.finishedOpenning();
}

void Container::ContainerHandler::on_container_stop(container&)
{
   std::cout << "client on_container_stop" << std::endl;
   m_manager.finishedClosing();
}
