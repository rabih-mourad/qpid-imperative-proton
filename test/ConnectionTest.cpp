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
- can create multiple sessions
- can't create sessions after close
- error handling correctly propagated
*/

TEST(ConnectionTest, connectionClosesCorrectlyOnDestruction)
{
   Broker brk("//127.0.0.1:5672", "examples");
   {
      proton::Container cont;
      proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   }
   std::cout << "Destroyed correctly " << std::endl;
}

TEST(ConnectionTest, ConnectionUnusableIfConnnectionCloses)
{
   Broker brk("//127.0.0.1:5672", "examples");

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   conn.close();
   try {
      std::cout << "opening session" << std::endl;
      conn.openSession(proton::session_options());
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(ConnectionTest, ConnectionClosesIfContainerCloses)
{
   Broker brk("//127.0.0.1:5672", "examples");

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   cont.close();
   try {
      std::cout << "opening session" << std::endl;
      conn.openSession(proton::session_options());
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(ConnectionTest, errorHandlingAtStart)
{
   try {
      proton::Container().openConnection("//127.0.0.1:5673", proton::connection_options());
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cerr << "Expected exception: " << e.what() << std::endl;
   }
}
