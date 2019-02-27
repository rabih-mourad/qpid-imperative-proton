#ifndef TEST_BROKER_HPP
#define TEST_BROKER_HPP

#include <proton/listener.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/listen_handler.hpp>

#include <proton/imperative/ThreadRAII.hpp>

#include <string>
#include <iostream>
#include <thread>
#include <future>

class listenerHandler : public proton::listen_handler
{
public:
   std::future<void> getStartedFuture()
   {
      return m_containerStarted.get_future();
   }

private:
   void on_open(proton::listener&) override
   {
      m_containerStarted.set_value();
   }

   void on_error(proton::listener&, const std::string& s) override
   {
      m_containerStarted.set_exception(std::make_exception_ptr(std::runtime_error(s)));
   }

   std::promise<void> m_containerStarted;
};

class Broker : public proton::messaging_handler
{
public:
   Broker(const std::string& url, const std::string& destination)
      :m_url(url + "/" + destination)
   {
      m_brokerThread = std::thread([&]() {
         try
         {
            proton::container(*this).run();
         }
         catch (const std::exception& e) {
            std::cerr << "Broker threw exception: " << e.what() << std::endl;
         }});

      // Wait for the container to start
      m_listenerHandler.getStartedFuture().get();
   }

   void injectMessages(std::vector<proton::message> messages)
   {
      m_messages.insert(m_messages.end(), messages.begin(), messages.end());
   }

private:
   void on_container_start(proton::container &c)
   {
      std::cout << "broker on_container_start" << std::endl;
      c.receiver_options(proton::receiver_options());
      m_listener = c.listen(m_url, m_listenerHandler);
   }

   void on_connection_open(proton::connection &c)
   {
      std::cout << "broker on_connection_open" << std::endl;
      m_connection = c;
      c.open();
   }

   void on_connection_close(proton::connection&)
   {
      std::cout << "broker on_connection_close" << std::endl;
      m_listener.stop();
   }

   void on_transport_error(proton::transport &t)
   {
      std::cout << "broker on_transport_error" << std::endl;
      std::cerr << "Broker::on_transport_error: " << t.error().what() << std::endl;
      m_listener.stop();
   }

   void on_error(const proton::error_condition &c)
   {
      std::cout << "broker on_error" << std::endl;
      std::cerr << "Broker::on_error: " << c.what() << std::endl;
      m_listener.stop();
   }

   void on_message(proton::delivery& delivery, proton::message& message)
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

   void on_sendable(proton::sender& sender)
   {
      std::cout << "broker on_sendable" << std::endl;
      sendMessages(sender);
   }

private:
   std::string m_url;
   proton::ThreadRAII m_brokerThread;
   std::deque<proton::message> m_messages;
   std::pair<proton::message, proton::delivery> m_currentDelivery;
   listenerHandler m_listenerHandler;

   proton::listener m_listener;
   proton::connection m_connection;
};

#endif
