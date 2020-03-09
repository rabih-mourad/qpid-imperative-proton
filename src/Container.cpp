#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>

#include <Logger.hpp>

using namespace proton;

namespace {
   Log debug_log;
}

static std::string containerCloseMsg("Container explicitly closed");

Container::Container()
   :m_container(m_containerHandler), m_pnErr(containerCloseMsg)
{
   auto f = m_containerHandler.m_manager.getOpenFuture();
   m_thread = ThreadRAII(std::thread([&]() {
      try {
         // The autostop flag should be returned to false when container.stop is thread safe
         m_container.auto_stop(true);
         m_container.run();
      } catch (const std::exception& e) {
         debug_log << "std::exception caught on pn container, message:" << e.what() << std::endl;
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
   debug_log << "---Container close called" << std::endl;
   if (!m_containerHandler.m_manager.hasBeenClosed() && !m_containerHandler.m_manager.isInError())
   {
      // We get the future before launching the action because std promise is not thread safe between get and set
      auto f = m_containerHandler.m_manager.close();

      // This condition should be removed when container.stop is back to be thread safe
      // If container still have active connections or didn't create any yet
      if (m_containerHandler.m_manager.getCloseRegistry()->getNumberOfRegisteredCallbacks() > 0 || firstConnCreated == false) {
         m_container.stop(m_pnErr);
      }
      lock.unlock();
      debug_log << "---Container close is waiting" << std::endl;
      f.get();
      debug_log << "---Container finished close" << std::endl;
   }
}

Connection Container::openConnection(const std::string& url, connection_options conn_opts)
{
   m_containerHandler.m_manager.checkForExceptions();
   Connection conn(m_container, url, conn_opts, m_containerHandler.m_manager.getCloseRegistry());
   firstConnCreated = true;
   conn.getOpenFuture().get();
   return conn;
}

//ContainerHandler

Container::ContainerHandler::ContainerHandler()
   :m_manager(nullptr, [](const std::string&){})
{}

void Container::ContainerHandler::on_container_start(proton::container&)
{
   debug_log << "client on_container_start" << std::endl;
   m_manager.handlePnOpen();
}

void Container::ContainerHandler::on_container_stop(container&)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   debug_log << "client on_container_stop" << std::endl;
   m_manager.handlePnClose();
}
