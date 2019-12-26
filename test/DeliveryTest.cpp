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
- test auto ack if manually acked
- can't ack after close
- can't ack after error
- error handling correctly propagated
- receiving is async
- ack testing
- ack double
- test no only accept in close
*/

TEST(DeliveryTest, canAcknowledgeInAsynch)
{
   Broker brk(url, destination);
   std::vector<proton::message> msgs{ proton::message("msg1"), proton::message("msg2"), proton::message("msg3"), proton::message("msg4"), proton::message("msg5"), proton::message("msg6") };
   brk.injectMessages(msgs);
   {
      proton::Container cont;
      proton::Connection conn = cont.openConnection(url, proton::connection_options());
      proton::Session sess = conn.openSession(proton::session_options());
      proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options().auto_accept(false));

      proton::Delivery del1 = rec.receive().get();
      proton::Delivery del2 = rec.receive().get();
      proton::Delivery del3 = rec.receive().get();
      proton::Delivery del4 = rec.receive().get();
      proton::Delivery del5 = rec.receive().get();
      proton::Delivery del6 = rec.receive().get();
      del2.reject();
      del3.release();
      del1.accept();
      del4.accept();
      del5.accept();
      del6.reject();
   }

   ASSERT_EQ(3, brk.m_acceptedMsgs);
   ASSERT_EQ(2, brk.m_rejectedMsgs);
   ASSERT_EQ(1, brk.m_releasedMsgs);
}

TEST(DeliveryTest, oneShotReceiver)
{
   Broker brk(url, destination);
   const std::string msgStr("my message");
   std::vector<proton::message> msgs{ proton::message(msgStr) };
   brk.injectMessages(msgs);

   proton::message msg = proton::Container()
                           .openConnection(url, proton::connection_options())
                           .openSession(proton::session_options())
                           .openReceiver(destination, proton::receiver_options().auto_accept(false))
                           .receive().get().pn_message;

   std::string receivedMsg(proton::get<std::string>(msg.body()));

   ASSERT_STREQ(msgStr.c_str(), receivedMsg.c_str());
}

TEST(DeliveryTest, DISABLED_acknowledgeWithAutoacceptDoesNothing)
{
   Broker brk(url, destination);
   std::vector<proton::message> msgs{ proton::message("msg1")};
   brk.injectMessages(msgs);
   {
      proton::Container cont;
      proton::Connection conn = cont.openConnection(url, proton::connection_options());
      proton::Session sess = conn.openSession(proton::session_options());
      proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options().auto_accept(true));

      proton::Delivery del1 = rec.receive().get();
      del1.reject();
   }

   ASSERT_EQ(1, brk.m_acceptedMsgs);
   ASSERT_EQ(0, brk.m_rejectedMsgs);
}

TEST(DeliveryTest, deliveryUnusableAfterCloseIsCalledOnReceiver)
{
   Broker brk(url, destination);
   std::vector<proton::message> msgs{ proton::message("msg1") };
   brk.injectMessages(msgs);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options());
   std::future<proton::Delivery> awaitrec = rec.receive();
   proton::Delivery del = awaitrec.get();
   rec.close();
   try {
      del.accept();
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(DeliveryTest, deliveryUnusableAfterCloseIsCalledOnSession)
{
   Broker brk(url, destination);
   std::vector<proton::message> msgs{ proton::message("msg1") };
   brk.injectMessages(msgs);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options().auto_accept(false));
   std::future<proton::Delivery> awaitrec = rec.receive();
   proton::Delivery del = awaitrec.get();
   sess.close();
   try {
      del.accept();
      FAIL() << "Test should fail if send does not throw";
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(DeliveryTest, deliveryUnusableAfterCloseIsCalledOnConnection)
{
   Broker brk(url, destination);
   std::vector<proton::message> msgs{ proton::message("msg1") };
   brk.injectMessages(msgs);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options().auto_accept(false));
   std::future<proton::Delivery> awaitrec = rec.receive();
   proton::Delivery del = awaitrec.get();
   conn.close();
   try {
      del.accept();
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(DeliveryTest, deliveryUnusableAfterCloseIsCalledOnContainer)
{
   Broker brk(url, destination);
   std::vector<proton::message> msgs{ proton::message("msg1") };
   brk.injectMessages(msgs);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options().auto_accept(false));
   std::future<proton::Delivery> awaitrec = rec.receive();
   proton::Delivery del = awaitrec.get();
   cont.close();
   try {
      del.accept();
      FAIL() << TestShouldFailIfNoThrow;
   }
   catch (std::exception& e) {
      ASSERT_STREQ(ParentObjectWasClosedExceptionMsg.c_str(), e.what());
   }
}

TEST(DeliveryTest, deliveryInErrorIfConnectionInError)
{
   Broker brk(url, destination);
   std::vector<proton::message> msgs{ proton::message("msg1") };
   brk.injectMessages(msgs);

   proton::Container cont;
   proton::Connection conn = cont.openConnection(url, proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver(destination, proton::receiver_options());
   std::future<proton::Delivery> awaitrec = rec.receive();
   proton::Delivery del = awaitrec.get();

   const std::string errMsg("Simulating network error");
   brk.close(errMsg);
   std::this_thread::sleep_for(std::chrono::milliseconds(4));
   // TO DO might be random if the on_error is called lately
   EXPECT_THROW(del.accept(), std::exception);
}
