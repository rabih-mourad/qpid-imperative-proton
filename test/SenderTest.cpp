#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/message.hpp>
#include <proton/connection_options.hpp>
#include <proton/session_options.hpp>
#include <proton/sender_options.hpp>
#include <proton/reconnect_options.hpp>

#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>
#include <proton/imperative/Session.hpp>
#include <proton/imperative/Sender.hpp>

#include <Broker.hpp>

#include <iostream>

#include <gtest/gtest.h>

/*
tests to add:
- can't send after close
- error handling correctly propagated
- sending is async
- destroy message just after send will not crash
- if in error release all promises
*/

TEST(SenderTest, canSendInAsynch)
{
   Broker brk("//127.0.0.1:5672", "examples");

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Sender sen = sess.openSender("examples", proton::sender_options());

   proton::message msg("test");

   std::future<void> awaitsent1 = sen.send(msg);
   std::future<void> awaitsent2 = sen.send(msg);
   awaitsent2.get();
   awaitsent1.get();

   sen.send(msg).get();

   sen.close();
   sess.close();
   conn.close();
   cont.close();
}

TEST(SenderTest, oneShotSender)
{
   Broker brk("//127.0.0.1:5672", "examples");

   proton::Container()
      .openConnection("//127.0.0.1:5672", proton::connection_options())
      .openSession(proton::session_options())
      .openSender("examples", proton::sender_options())
      .send(proton::message("test"))
      .get();
}

TEST(SenderTest, senderUnusableAfterCloseIsCalled)
{
   Broker brk("//127.0.0.1:5672", "examples");

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Sender sen = sess.openSender("examples", proton::sender_options());
   sen.getOpenFuture().get();
   sen.close();
   try {
      proton::message msg("test");
      sen.send(msg).get();
      FAIL() << "Test should fail if send does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(SenderTest, DISABLED_senderClosesIfConnectionCloses)
{
   const std::string destination("examples");
   Broker brk("//127.0.0.1:5672", destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Sender sen = sess.openSender("examples", proton::sender_options());
   sen.getOpenFuture().get();
   conn.close();
   try {
      proton::message msg("test");
      sen.send(msg).get();
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(SenderTest, DISABLED_senderClosesIfContainerCloses)
{
   const std::string destination("examples");
   Broker brk("//127.0.0.1:5672", destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Sender sen = sess.openSender("examples", proton::sender_options());
   sen.getOpenFuture().get();
   cont.close();
   try {
      proton::message msg("test");
      sen.send(msg).get();
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(SenderTest, DISABLED_errorHandlingAtSend)
{
   Broker brk("//127.0.0.1:5672", "examples");
   //should simulate an error when send in broker
   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", 
      proton::connection_options().idle_timeout(proton::duration(2000))
      .reconnect(proton::reconnect_options().max_attempts(50).delay(proton::duration(50)).delay_multiplier(1).max_delay(proton::duration(1000))));

   proton::Session sess = conn.openSession(proton::session_options());

   proton::Sender sen = sess.openSender("examples", proton::sender_options());
   sen.getOpenFuture().get();

   //Simulate Connection Error

   try {
      proton::message msg("test");
      sen.send(msg).get();
      FAIL() << "Test should fail if send does not throw";
   }
   catch (std::exception& e) {
      std::cerr << "Expected exception: " << e.what() << std::endl;
   }

   try {
      proton::Session sess1 = conn.openSession(proton::session_options());
      sess1.getOpenFuture().get();
      FAIL() << "Test should fail if send does not throw";
   }
   catch (std::exception& e) {
      std::cerr << "Expected exception: " << e.what() << std::endl;
   }
}
