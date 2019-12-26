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
#include <Constants.hpp>

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

TEST(SenderTest, senderClosesCorrectlyOnDestruction)
{
   Broker brk(url, destination);
   {
      proton::Container cont;
      proton::Connection conn = cont.openConnection(url, proton::connection_options());
      proton::Session sess = conn.openSession(proton::session_options());
      proton::Sender sen = sess.openSender(destination, proton::sender_options());
      sen.getOpenFuture().get();
   }
   std::cout << "Destroyed correctly " << std::endl;
}

TEST(SenderTest, canSendInAsynch)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Sender sen = sess.openSender(destination, proton::sender_options());

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
   Broker brk(url, destination);

   proton::Container()
      .openConnection(url, proton::connection_options())
      .openSession(proton::session_options())
      .openSender(destination, proton::sender_options())
      .send(proton::message("test"))
      .get();
}

TEST(SenderTest, senderUnusableAfterCloseIsCalled)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Sender sen = sess.openSender(destination, proton::sender_options());
   sen.getOpenFuture().get();
   sen.close();
   try {
      proton::message msg("test");
      sen.send(msg).get();
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ObjectClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(SenderTest, senderClosesIfConnectionCloses)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Sender sen = sess.openSender(destination, proton::sender_options());
   sen.getOpenFuture().get();
   conn.close();
   try {
      proton::message msg("test");
      sen.send(msg).get();
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(SenderTest, senderClosesIfContainerCloses)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Sender sen = sess.openSender(destination, proton::sender_options());
   sen.getOpenFuture().get();
   cont.close();
   try {
      proton::message msg("test");
      sen.send(msg).get();
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(SenderTest, senderInErrorIfConnectionInError)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Sender sen = sess.openSender(destination, proton::sender_options());
   sen.getOpenFuture().get();

   const std::string errMsg("Simulating network error");
   brk.close(errMsg);
   std::this_thread::sleep_for(std::chrono::milliseconds(2));

   proton::message msg("test");
   EXPECT_THROW(sen.send(msg).get(), std::exception);

   EXPECT_THROW(conn.openSession(proton::session_options()).getOpenFuture().get(), std::exception);
}
