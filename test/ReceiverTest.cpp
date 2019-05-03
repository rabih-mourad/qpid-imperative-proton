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
- can't receive after close
- error handling correctly propagated
- receiving is async
- if in error release all promises
*/

TEST(ReceiverTest, canReceiveInAsynch)
{
   Broker brk("//127.0.0.1:5672", "examples");
   std::vector<proton::message> msgs{ proton::message("msg1"), proton::message("msg2"), proton::message("msg3") };
   brk.injectMessages(msgs);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options());

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
   sess.close();
   conn.close();
   cont.close();
}

TEST(ReceiverTest, oneShotReceiver)
{
   Broker brk("//127.0.0.1:5672", "examples");
   const std::string msgStr("my message");
   std::vector<proton::message> msgs{ proton::message(msgStr) };
   brk.injectMessages(msgs);

   proton::message msg = proton::Container()
                           .openConnection("//127.0.0.1:5672", proton::connection_options())
                           .openSession(proton::session_options())
                           .openReceiver("examples", proton::receiver_options())
                           .receive().get().pn_message;

   std::string receivedMsg(proton::get<std::string>(msg.body()));

   ASSERT_STREQ(msgStr.c_str(), receivedMsg.c_str());
}

TEST(ReceiverTest, receiverUnusableAfterCloseIsCalled)
{
   const std::string destination("examples");
   Broker brk("//127.0.0.1:5672", destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options());
   rec.getOpenFuture().get();
   rec.close();
   try {
      rec.receive();
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(ReceiverTest, DISABLED_ReceiverClosesIfSessionCloses)
{
   Broker brk("//127.0.0.1:5672", "examples");

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options().auto_accept(false));
   rec.getOpenFuture().get();
   sess.close();
   try {
      proton::message msg = rec.receive().get().pn_message;
      FAIL() << "Test should fail if send does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(ReceiverTest, DISABLED_receiverClosesIfConnectionCloses)
{
   const std::string destination("examples");
   Broker brk("//127.0.0.1:5672", destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options().auto_accept(false));
   rec.getOpenFuture().get();
   conn.close();
   try {
      proton::message msg = rec.receive().get().pn_message;
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(ReceiverTest, DISABLED_receiverClosesIfContainerCloses)
{
   const std::string destination("examples");
   Broker brk("//127.0.0.1:5672", destination);

   proton::Container cont;
   proton::Connection conn = cont.openConnection("//127.0.0.1:5672", proton::connection_options());
   proton::Session sess = conn.openSession(proton::session_options());
   proton::Receiver rec = sess.openReceiver("examples", proton::receiver_options().auto_accept(false));
   rec.getOpenFuture().get();
   cont.close();
   try {
      proton::message msg = rec.receive().get().pn_message;
      FAIL() << "Test should fail if it does not throw";
   }
   catch (std::exception& e) {
      std::cout << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(ReceiverTest, DISABLED_errorHandlingAtReceive)
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
