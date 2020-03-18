#ifndef TEST_BROKER_HPP
#define TEST_BROKER_HPP

#include <proton/container.hpp>
#include <proton/listener.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/listen_handler.hpp>
#include <proton/error_condition.hpp>

#include <Constants.hpp>

#include <string>
#include <map>
#include <iostream>
#include <thread>
#include <future>


class ThreadRAII
{
public:
   ThreadRAII() = default;
   ThreadRAII(ThreadRAII&& thread) = default;
   ThreadRAII& operator=(ThreadRAII&& thread) = default;
   ThreadRAII(const ThreadRAII& other) = delete;
   ThreadRAII& operator=(const ThreadRAII& other) = delete;

   ThreadRAII(std::thread&& thread)
      : m_thread(std::move(thread))
   {
   }

   std::thread& get()
   {
      return m_thread;
   }

   ~ThreadRAII()
   {
      if (m_thread.joinable())
      {
         m_thread.join();
      }
   }

private:
   std::thread m_thread;
};

class ListenerHandler : public proton::listen_handler
{
public:
   ListenerHandler()
      :m_containerStarted(new std::promise<int>)
   {}

   std::future<int> getStartedFuture()
   {
      return m_containerStarted->get_future();
   }

   proton::listener m_listener;

private:
   void on_open(proton::listener& l) override
   {
      if (m_containerStarted)
      {
         m_containerStarted->set_value(l.port());
         m_containerStarted.reset();
      }
   }

   void on_error(proton::listener&, const std::string& s) override
   {
      if (m_containerStarted)
      {
         m_containerStarted->set_exception(std::make_exception_ptr(std::runtime_error(s)));
         m_containerStarted.reset();
      }
   }

   void on_close(proton::listener&) override
   {
      m_listener = proton::listener();
   }

   std::unique_ptr<std::promise<int>> m_containerStarted;
};

class Broker : public proton::messaging_handler
{
public:
   // Returning a pointer because Broker contains a proton::container which is not movable 
   static std::unique_ptr<Broker> createBrokerOnSpecifiedPort(int port)
   {
      return std::unique_ptr<Broker>(new Broker(host, port, destination));
   }

   static std::unique_ptr<Broker> createBroker()
   {
      //using port 0 and then caching the real port number to be able to launch multiple tests in parallel
      static int cachedPort = 0;
      std::unique_ptr<Broker> broker_ptr(new Broker(host, cachedPort, destination));
      if (cachedPort == 0) {
         cachedPort = broker_ptr->getPort();
      }
      return broker_ptr;
   }

   ~Broker()
   {
      if (!m_isClosed) {
         m_container.stop();
         m_stopPromise.get_future().wait();
         m_isClosed = true;
      }
   }

   void injectMessages(std::vector<proton::message> messages)
   {
      m_messages.insert(m_messages.end(), messages.begin(), messages.end());
   }

   std::string getURL()
   {
      return std::string().append(host).append(":").append(std::to_string(m_port));
   }

   int getPort()
   {
      return m_port;
   }

   // Call only if you have one connection to the broker
   void close(const std::string& errMsg)
   {
      if (!m_isClosed) {
         m_container.stop(errMsg);
         m_stopPromise.get_future().wait();
         m_isClosed = true;
      }
   }

   int m_acceptedMsgs = 0;
   int m_rejectedMsgs = 0;
   int m_releasedMsgs = 0;

private:
   Broker(const std::string& host, int port, const std::string& destination)
      :m_url(std::string().append(host).append(":").append(std::to_string(port)).append("/").append(destination)), m_container(*this)
   {
      auto future = m_listenerHandler.getStartedFuture();
      m_brokerThread = std::thread([&]() {
         try
         {
            m_container.run();
            std::cout << "Broker exiting event loop" << std::endl;
         }
         catch (const std::exception& e) {
            std::cerr << "Broker threw exception: " << e.what() << std::endl;
         }});

      // Wait for the container to start
      m_port = future.get();
   }

   void on_container_start(proton::container&) override
   {
      std::cout << "broker on_container_start" << std::endl;
      m_container.receiver_options(proton::receiver_options());
      m_listenerHandler.m_listener = m_container.listen(m_url, m_listenerHandler);
   }

   void on_container_stop(proton::container &c) override
   {
      std::cout << "broker on_container_stop" << std::endl;
      m_stopPromise.set_value();
   }

   void on_connection_open(proton::connection &c) override
   {
      std::cout << "broker on_connection_open" << std::endl;
      c.open();
   }

   void on_connection_close(proton::connection& c) override
   {
      std::cout << "broker on_connection_close" << std::endl;
   }

   void on_transport_error(proton::transport &t) override
   {
      std::cout << "broker on_transport_error" << std::endl;
      std::cerr << "Broker::on_transport_error: " << t.error().what() << std::endl;
   }

   void on_error(const proton::error_condition &c) override
   {
      //TO DO just log in this method
      std::cout << "broker on_error" << std::endl;
      std::cerr << "Broker::on_error: " << c.what() << std::endl;

      m_isClosed = true;
      m_listenerHandler.m_listener.stop();
   }

   void on_message(proton::delivery& delivery, proton::message& message) override
   {
      std::cout << "broker on_message" << std::endl;
      m_currentDelivery = { message, delivery };
   }

   void sendMessages(proton::sender& sender)
   {
      size_t numberOfMessagesToSend = std::min(static_cast<size_t>(sender.credit()), m_messages.size());

      for (auto count = numberOfMessagesToSend; count > 0; --count)
      {
         auto message = m_messages.front();
         m_messages.pop_front();
         sender.send(message);
      }
   }

   void on_sendable(proton::sender& sender) override
   {
      std::cout << "broker on_sendable" << std::endl;
      sendMessages(sender);
   }

   void on_tracker_accept(proton::tracker& ) override
   {
      ++m_acceptedMsgs;
   }

   void on_tracker_reject(proton::tracker&) override
   {
      ++m_rejectedMsgs;
   }

   void on_tracker_release(proton::tracker&) override
   {
      ++m_releasedMsgs;
   }

private:
   std::string m_url;
   int m_port;
   bool m_isClosed = false;
   proton::container m_container;
   ThreadRAII m_brokerThread;
   std::deque<proton::message> m_messages;
   std::pair<proton::message, proton::delivery> m_currentDelivery;
   ListenerHandler m_listenerHandler;

   std::promise<void> m_stopPromise;
};

#endif
