#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/session.hpp>
#include <proton/session_options.hpp>

#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>

#include <Broker.hpp>
#include <Constants.hpp>

#include <iostream>

#include <gtest/gtest.h>


TEST(SessionTest, sessionClosesCorrectlyOnDestruction)
{
   Broker brk(url, destination);
   {
      proton::Container cont;
      proton::Connection conn = cont.openConnection(url, proton::connection_options());
      proton::Session sess = conn.openSession(proton::session_options());
      sess.getOpenFuture().get();
   }
   std::cout << "Destroyed correctly " << std::endl;
}

TEST(SessionTest, CanCreateMultipleReceiversFromSession)
{
   Broker brk(url, destination);
   
   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec1 = sess.openReceiver(destination, proton::receiver_options());
   proton::Receiver rec2 = sess.openReceiver(destination, proton::receiver_options());
   rec1.getOpenFuture().get();
   rec2.getOpenFuture().get();

   rec1.close();
   rec2.close();
   sess.close();
   conn.close();
   cont.close();
}

TEST(SessionTest, CanCreateMultipleSendersFromSession)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Sender sen1 = sess.openSender(destination, proton::sender_options());
   proton::Sender sen2 = sess.openSender(destination, proton::sender_options());
   sen1.getOpenFuture().get();
   sen2.getOpenFuture().get();

   sen1.close();
   sen2.close();
   sess.close();
   conn.close();
   cont.close();
}

TEST(SessionTest, sessionUnusableIfCloseIsCalled)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   sess.getOpenFuture().get();
   sess.close();
   try {
      sess.openSender(destination, proton::sender_options());
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ObjectClosedExceptionMsg.c_str(), e.what());
   }

   try {
      sess.openReceiver(destination, proton::receiver_options());
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ObjectClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(SessionTest, sessionClosesIfConnectionCloses)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   sess.getOpenFuture().get();
   conn.close();
   try {
      sess.openSender(destination, proton::sender_options());
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(SessionTest, sessionClosesIfContainerCloses)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   sess.getOpenFuture().get();
   cont.close();
   try {
      sess.openSender(destination, proton::sender_options());
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(SessionTest, sessionInErrorIfConnectionInError)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   sess.getOpenFuture().get();

   const std::string errMsg("Simulating network error");
   brk.close(errMsg);

   std::this_thread::sleep_for(std::chrono::milliseconds(2));
   EXPECT_THROW(sess.openSender(destination, proton::sender_options()).getOpenFuture().get(), std::exception);
}
