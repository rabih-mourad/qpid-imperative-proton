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
   Broker brk("//127.0.0.1:5672", "examples");
   std::vector<proton::message> msgs{ proton::message("msg1"), proton::message("msg2"), proton::message("msg3"), proton::message("msg4"), proton::message("msg5"), proton::message("msg6") };
   brk.injectMessages(msgs);
   {
      proton::Container cont;
      proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
      proton::Session sess = conn.openSession(proton::session_options());
      proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options().auto_accept(false));

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
   Broker brk("//127.0.0.1:5672", "examples");
   const std::string msgStr("my message");
   std::vector<proton::message> msgs{ proton::message(msgStr) };
   brk.injectMessages(msgs);

   proton::message msg = proton::Container()
                           .openConnection("//127.0.0.1:5672", proton::connection_options())
                           .openSession(proton::session_options())
                           .openReceiver("examples", proton::receiver_options().auto_accept(false))
                           .receive().get().pn_message;

   std::string receivedMsg(proton::get<std::string>(msg.body()));

   ASSERT_STREQ(msgStr.c_str(), receivedMsg.c_str());
}

TEST(DeliveryTest, DISABLED_acknowledgeWithAutoacceptDoesNothing)
{
   Broker brk("//127.0.0.1:5672", "examples");
   std::vector<proton::message> msgs{ proton::message("msg1")};
   brk.injectMessages(msgs);
   {
      proton::Container cont;
      proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
      proton::Session sess = conn.openSession(proton::session_options());
      proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options().auto_accept(true));

      proton::Delivery del1 = rec.receive().get();
      del1.reject();
   }

   ASSERT_EQ(1, brk.m_acceptedMsgs);
   ASSERT_EQ(0, brk.m_rejectedMsgs);
}

TEST(DeliveryTest, DISABLED_receiverUnusableAfterCloseIsCalled)
{
   const std::string destination("examples");
   Broker brk("//127.0.0.1:5672", destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options());
   std::future<proton::Delivery> awaitrec = rec.receive();
   proton::Delivery del = awaitrec.get();
   rec.close();
   try {
      del.accept();
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(DeliveryTest, DISABLED_ReceiverClosesIfSessionCloses)
{
   Broker brk("//127.0.0.1:5672", "examples");

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options().auto_accept(false));
   std::future<proton::Delivery> awaitrec = rec.receive();
   proton::Delivery del = awaitrec.get();
   sess.close();
   try {
      del.accept();
      FAIL() << "Test should fail if send does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(DeliveryTest, DISABLED_receiverClosesIfConnectionCloses)
{
   const std::string destination("examples");
   Broker brk("//127.0.0.1:5672", destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options().auto_accept(false));
   std::future<proton::Delivery> awaitrec = rec.receive();
   proton::Delivery del = awaitrec.get();
   conn.close();
   try {
      del.accept();
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(DeliveryTest, DISABLED_receiverClosesIfContainerCloses)
{
   const std::string destination("examples");
   Broker brk("//127.0.0.1:5672", destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options().auto_accept(false));
   std::future<proton::Delivery> awaitrec = rec.receive();
   proton::Delivery del = awaitrec.get();
   cont.close();
   try {
      del.accept();
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(DeliveryTest, DISABLED_errorHandlingAtReceive)
{
   Broker brk("//127.0.0.1:5672", "examples");
   //should simulate an error when send in broker
   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options().idle_timeout(proton::duration(2000)).reconnect(proton::reconnect_options().max_attempts(50).delay(proton::duration(50)).delay_multiplier(1).max_delay(proton::duration(1000))));
   std::cout << "connection get" << std::endl;

   proton::Session sess = conn.openSession(proton::session_options());
   sess.getOpenFuture().get();
   std::cout << "session get" << std::endl;

   proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options().auto_accept(false));
   rec.getOpenFuture().get();
   std::cout << "receiver get" << std::endl;

   try {
      std::future<proton::Delivery> awaitrec1 = rec.receive();
      awaitrec1.get();
      std::cout << "message get" << std::endl;
   }
   catch (std::exception& e) {
      std::cerr << "Expected exception: " << e.what() << std::endl;
   }

   try {
      proton::Session sess1 = conn.openSession(proton::session_options());
      sess1.getOpenFuture().get();
   }
   catch (std::exception& e) {
      std::cerr << "Expected exception: " << e.what() << std::endl;
   }
}
