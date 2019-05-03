#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/session.hpp>
#include <proton/session_options.hpp>

#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>

#include <Broker.hpp>

#include <iostream>

#include <gtest/gtest.h>

/*
tests to add:
- can create multiple receivers
- can create multiple senders
- can't create sessions after close
- error handling correctly propagated
*/

TEST(SessionTest, canCreateSessionFromContainer)
{
   Broker brk("//127.0.0.1:5672", "examples");

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   sess.getOpenFuture().get();

   sess.close();
   conn.close();
   cont.close();
}

TEST(SessionTest, sessionClosesCorrectlyOnDestruction)
{
   Broker brk("//127.0.0.1:5672", "examples");
   {
      proton::Container cont;
      proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
      proton::Session sess = conn.openSession(proton::session_options());
      sess.getOpenFuture().get();
   }
   std::cout << "Destroyed correctly " << std::endl;
}

TEST(SessionTest, sessionUnusableIfCloseIsCalled)
{
   const std::string destination("examples");
   Broker brk("//127.0.0.1:5672", destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   sess.getOpenFuture().get();
   sess.close();
   try {
      sess.openSender(destination, proton::sender_options());
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }

   try {
      sess.openReceiver(destination, proton::receiver_options());
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(SessionTest, DISABLED_sessionClosesIfConnectionCloses)
{
   const std::string destination("examples");
   Broker brk("//127.0.0.1:5672", destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   sess.getOpenFuture().get();
   conn.close();
   try {
      sess.openSender(destination, proton::sender_options());
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(SessionTest, DISABLED_sessionClosesIfContainerCloses)
{
   const std::string destination("examples");
   Broker brk("//127.0.0.1:5672", destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   sess.getOpenFuture().get();
   cont.close();
   try {
      sess.openSender(destination, proton::sender_options());
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}
