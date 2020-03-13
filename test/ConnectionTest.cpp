#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>

#include <Constants.hpp>
#include <Broker.hpp>
#include <ProtonIncludes.hpp>

#include <iostream>

#include <gtest/gtest.h>


TEST(ConnectionTest, connectionClosesCorrectlyOnDestruction)
{
   Broker brk(url, destination);
   {
      proton::Container cont;
      proton::Connection conn = cont.openConnection(url, proton::connection_options());
   }
   std::cout << "Destroyed correctly " << std::endl;
}

TEST(ConnectionTest, CanCreateMultipleSessionsFromConnection)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess1 = conn.openSession(proton::session_options());
   proton::Session sess2 = conn.openSession(proton::session_options());
   sess1.getOpenFuture().get();
   sess2.getOpenFuture().get();
   sess1.close();
   sess2.close();
   conn.close();
   cont.close();
}

TEST(ConnectionTest, ConnectionUnusableIfConnnectionCloses)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   conn.close();
   try {
      std::cout << "opening session" << std::endl;
      conn.openSession(proton::session_options());
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ObjectClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(ConnectionTest, ConnectionClosesIfContainerCloses)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   cont.close();
   try {
      std::cout << "opening session" << std::endl;
      conn.openSession(proton::session_options());
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(ConnectionTest, errorHandlingAtStart)
{
   try {
      proton::Container cont;
      std::cout << "---container created" << std::endl;
      proton::Connection conn = cont.openConnection("//127.0.0.1:5673", proton::connection_options());
      std::cout << "---connection created" << std::endl;
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      std::cerr << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(ConnectionTest, connectionThrowsCorrectError)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());

   const std::string errMsg("Simulating network error");
   brk.close(errMsg);
   std::this_thread::sleep_for(std::chrono::milliseconds(2));
   EXPECT_THROW(conn.openSession(proton::session_options()).getOpenFuture().get(), std::exception);
}
