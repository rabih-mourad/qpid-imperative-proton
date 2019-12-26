#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>

#include <iostream>

using namespace proton;

static std::string containerCloseMsg("Container explicitly closed");

Container::Container()
   :m_container(m_containerHandler), m_pnErr(containerCloseMsg)
{
   auto f = m_containerHandler.m_manager.getOpenFuture();
   m_thread = ThreadRAII(std::thread([&]() {
      try {
         m_container.auto_stop(false);
         m_container.run();
      } catch (const std::exception& e) {
         std::cout << "std::exception caught on pn container, message:" << e.what() << std::endl;
      }}));
   f.get();
}

Container::~Container()
{
   close();
}

void Container::close()
{
   std::unique_lock<std::mutex> lock(m_containerHandler.m_manager.m_mutex);
   std::cout << "---Container close called" << std::endl;
   if (!m_containerHandler.m_manager.hasBeenClosed() && !m_containerHandler.m_manager.isInError())
   {
      // We get the future before launching the action because std promise is not thread safe between get and set
      auto f = m_containerHandler.m_manager.close();
      m_container.stop(m_pnErr);
      lock.unlock();
      std::cout << "---Container close is waiting" << std::endl;
      f.get();
      std::cout << "---Container finished close" << std::endl;
   }
}

Connection Container::openConnection(const std::string& url, connection_options conn_opts)
{
   m_containerHandler.m_manager.checkForExceptions();
   Connection conn(m_container, url, conn_opts, m_containerHandler.m_manager.getCloseRegistry());
   conn.getOpenFuture().get();
   return conn;
}

//ContainerHandler

Container::ContainerHandler::ContainerHandler()
   :m_manager(nullptr, [](const std::string&){})
{}

void Container::ContainerHandler::on_container_start(proton::container&)
{
   std::cout << "client on_container_start" << std::endl;
   m_manager.handlePnOpen();
}

void Container::ContainerHandler::on_container_stop(container&)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   std::cout << "client on_container_stop" << std::endl;
   m_manager.handlePnClose();
}
