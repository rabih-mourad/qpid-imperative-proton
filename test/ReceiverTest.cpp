#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/delivery.hpp>
#include <proton/message.hpp>
#include <proton/connection_options.hpp>
#include <proton/session_options.hpp>
#include <proton/reconnect_options.hpp>

#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>
#include <proton/imperative/Session.hpp>
#include <proton/imperative/Receiver.hpp>
#include <proton/imperative/Delivery.hpp>

#include <Broker.hpp>
#include <Constants.hpp>

#include <iostream>
#include <vector>

#include <gtest/gtest.h>

/*
tests to add:
- can't receive after close
- error handling correctly propagated
- receiving is async
- if in error release all promises
*/

TEST(ReceiverTest, receiverClosesCorrectlyOnDestruction)
{
   Broker brk(url, destination);
   {
      proton::Container cont;
      proton::Connection conn = cont.openConnection(url, proton::connection_options());
      proton::Session sess = conn.openSession(proton::session_options());
      proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options());
      rec.getOpenFuture().get();
   }
   std::cout << "Destroyed correctly " << std::endl;
}

TEST(ReceiverTest, canReceiveInAsynch)
{
   Broker brk(url, destination);
   std::vector<proton::message> msgs{ proton::message("msg1"), proton::message("msg2"), proton::message("msg3") };
   brk.injectMessages(msgs);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options());

   std::future<proton::Delivery> awaitrec1 = rec.receive();
   std::future<proton::Delivery> awaitrec2 = rec.receive();
   std::future<proton::Delivery> awaitrec3 = rec.receive();

   proton::Delivery del2 = awaitrec2.get();
   proton::Delivery del3 = awaitrec3.get();
   proton::Delivery del1 = awaitrec1.get();
   ASSERT_STREQ(proton::get<std::string>(msgs[0].body()).c_str(), proton::get<std::string>(del1.pn_message.body()).c_str());
   ASSERT_STREQ(proton::get<std::string>(msgs[1].body()).c_str(), proton::get<std::string>(del2.pn_message.body()).c_str());
   ASSERT_STREQ(proton::get<std::string>(msgs[2].body()).c_str(), proton::get<std::string>(del3.pn_message.body()).c_str());

   rec.close();
   std::cout << "---Connection releasePnMemberObjects" << std::endl;
   sess.close();
   conn.close();
   cont.close();
}

TEST(ReceiverTest, oneShotReceiver)
{
   Broker brk(url, destination);
   const std::string msgStr("my message");
   std::vector<proton::message> msgs{ proton::message(msgStr) };
   brk.injectMessages(msgs);

   proton::message msg = proton::Container()
                           .openConnection(url, proton::connection_options())
                           .openSession(proton::session_options())
                           .openReceiver(destination, proton::receiver_options())
                           .receive().get().pn_message;

   std::string receivedMsg(proton::get<std::string>(msg.body()));

   ASSERT_STREQ(msgStr.c_str(), receivedMsg.c_str());
}

TEST(ReceiverTest, receiverUnusableAfterCloseIsCalled)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options());
   rec.getOpenFuture().get();
   rec.close();
   try {
      rec.receive();
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ObjectClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(ReceiverTest, receiverClosesIfSessionCloses)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options().auto_accept(false));
   rec.getOpenFuture().get();
   sess.close();
   try {
      proton::message msg = rec.receive().get().pn_message;
      FAIL() << "Test should fail if send does not throw";
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(ReceiverTest, receiverClosesIfConnectionCloses)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options().auto_accept(false));
   rec.getOpenFuture().get();
   conn.close();
   try {
      proton::message msg = rec.receive().get().pn_message;
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(ReceiverTest, receiverClosesIfContainerCloses)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options().auto_accept(false));
   rec.getOpenFuture().get();
   cont.close();
   try {
      proton::message msg = rec.receive().get().pn_message;
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(ReceiverTest, receiverInErrorIfConnectionInError)
{
   Broker brk(url, destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options());
   rec.getOpenFuture().get();

   const std::string errMsg("Simulating network error");
   brk.close(errMsg);
   std::this_thread::sleep_for(std::chrono::milliseconds(2));

   EXPECT_THROW(rec.receive().get(), std::exception);

   EXPECT_THROW(conn.openSession(proton::session_options()).getOpenFuture().get(), std::exception);
}
