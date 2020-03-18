#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>

#include <Logger.hpp>
#include <ThreadRAII.hpp>

#include <proton/connection_options.hpp>
#include <proton/container.hpp>
#include <proton/messaging_handler.hpp>

using namespace proton;

namespace {
   Log debug_log;
}

class Container::Impl : public messaging_handler {
public:
   Impl();

   Connection openConnection(const std::string& url, connection_options conn_opts);
   void close();

   ~Impl();

   Impl(const Impl& other) = delete;
   Impl& operator=(const Impl& other) = delete;
   Impl(Impl&& c) = default;
   Impl& operator=(Impl&& other) = default;

private:
   void on_container_start(proton::container&) override;
   void on_container_stop(container&) override;

private:
   PnObjectLifeManager m_manager;
   container m_container;
   ThreadRAII m_thread;

   // The following flag should be removed when container::stop becomes thread safe
   bool firstConnCreated = false;
};

Container::Impl::Impl()
   : m_manager(nullptr, [](const std::string&) {}),
     m_container(*this)
     
{
   auto containerStarted = m_manager.getOpenFuture();

   m_thread = std::thread([&]() {
      try {
         // The autostop flag should be set to false when container::stop becomes thread safe
         m_container.auto_stop(true);
         m_container.run();
      } catch (const std::exception& e) {
         debug_log << "std::exception caught on pn container, message:" << e.what() << std::endl;
      }
   });

   containerStarted.get();
}

Connection Container::Impl::openConnection(const std::string& url, connection_options conn_opts)
{
   m_manager.checkForExceptions();

   Connection conn(m_container, url, conn_opts, m_manager.getCloseRegistry());
   firstConnCreated = true;
   conn.getOpenFuture().get();
   return conn;
}

void Container::Impl::close()
{
   std::unique_lock<std::mutex> lock(m_manager.m_mutex);
   debug_log << "---Container close called" << std::endl;

   if (!m_manager.hasBeenClosed() && !m_manager.isInError())
   {
      // We get the future before launching the action because std promise is not thread safe between get and set
      auto containerStopped = m_manager.close();

      // This condition should be removed when container.stop is back to be thread safe
      // If container still have active connections or didn't create any yet
      if (m_manager.getCloseRegistry()->getNumberOfRegisteredCallbacks() > 0 || !firstConnCreated) {
         m_container.stop(error_condition{ "Container explicitly closed" });
      }

      lock.unlock();
      debug_log << "---Container close is waiting" << std::endl;
      containerStopped.get();
      debug_log << "---Container finished close" << std::endl;
   }
}

Container::Impl::~Impl()
{
   close();
}

void Container::Impl::on_container_start(proton::container&)
{
   debug_log << "client on_container_start" << std::endl;
   m_manager.handlePnOpen();
}

void Container::Impl::on_container_stop(container&)
{
   std::lock_guard<std::mutex> lock(m_manager.m_mutex);
   debug_log << "client on_container_stop" << std::endl;
   m_manager.handlePnClose();
}

Container::Container()
   : m_impl(new Container::Impl)
{
}

Connection Container::openConnection(const std::string& url, connection_options conn_opts)
{
   return m_impl->openConnection(url, conn_opts);
}

void Container::close()
{
   m_impl->close();
}
