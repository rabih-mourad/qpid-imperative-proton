#include <proton/connection.hpp>
#include <proton/container.hpp>
#include <proton/delivery.hpp>
#include <proton/message.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/tracker.hpp>
#include <proton/connection_options.hpp>
#include <proton/session_options.hpp>
#include <proton/sender_options.hpp>

#include <proton/imperative/Container.hpp>
#include <proton/imperative/Connection.hpp>
#include <proton/imperative/Session.hpp>
#include <proton/imperative/Sender.hpp>
#include <proton/imperative/Receiver.hpp>

#include <Broker.hpp>

#include <iostream>
#include <vector>

#include <gtest/gtest.h>

TEST(main, canSendInAsynch)
{
   try {
      Broker brk("//127.0.0.1:5672", "examples");
      proton::Container cont;
      proton::Connection conn = cont.createConnection();
      conn.open("//127.0.0.1:5672", proton::connection_options()).get();

      proton::Session sess = conn.createSession();
      sess.open(proton::session_options()).get();

      proton::Sender sen = sess.createSender();
      sen.open("examples", proton::sender_options()).get();

      proton::message msg("test");

      std::future<void> awaitsent1 = sen.send(msg);
      std::future<void> awaitsent2 = sen.send(msg);
      awaitsent1.get();
      awaitsent2.get();

      sen.send(msg).get();

      sen.close();
      sess.close();
      conn.close();
      cont.close();
      std::cout << "test done" << std::endl;
   }
   catch (std::exception& e) {
      std::cerr << "exception in main: " << e.what() << std::endl;
   }
   catch (...) {
      std::cerr << "unknown error" << std::endl;
   }
}

TEST(main, canReceiveInAsynch)
{
   Broker brk("//127.0.0.1:5672", "examples");
   std::vector<proton::message> msgs{ proton::message("msg1"), proton::message("msg2"), proton::message("msg3") };
   brk.injectMessages(msgs);
   try {
      proton::message msg("test");

      proton::Container cont;
      proton::Connection conn = cont.createConnection();
      conn.open("//127.0.0.1:5672", proton::connection_options()).get();

      proton::Session sess = conn.createSession();
      sess.open(proton::session_options()).get();

      proton::Receiver rec = sess.createReceiver();
      rec.open("examples", proton::receiver_options().auto_accept(false)).get();

      std::future<proton::Receiver::Delivery> awaitrec1 = rec.receive();
      std::future<proton::Receiver::Delivery> awaitrec2 = rec.receive();
      std::future<proton::Receiver::Delivery> awaitrec3 = rec.receive();

      proton::Receiver::Delivery del2 = awaitrec2.get();
      proton::Receiver::Delivery del3 = awaitrec3.get();
      proton::Receiver::Delivery del1 = awaitrec1.get();
      del1.reject();
      del3.accept();
      del2.release();
      std::cout << del1.pn_message.body() << std::endl;
      std::cout << del2.pn_message.body() << std::endl;
      std::cout << del3.pn_message.body() << std::endl;

      rec.close();
      sess.close();
      conn.close();
      cont.close();
      std::cout << "test done" << std::endl;
   }
   catch (std::exception& e) {
      std::cerr << "exception in main: " << e.what() << std::endl;
   }
   catch (...) {
      std::cerr << "unknown error" << std::endl;
   }
}

TEST(main, noClose)
{
   try {
      Broker brk("//127.0.0.1:5672", "examples");
      proton::Container cont;
      proton::Connection conn = cont.createConnection();
      conn.open("//127.0.0.1:5672", proton::connection_options()).get();

      proton::Session sess = conn.createSession();
      sess.open(proton::session_options()).get();

      proton::Sender sen = sess.createSender();
      sen.open("examples", proton::sender_options()).get();

      proton::Receiver rec = sess.createReceiver();
      rec.open("examples", proton::receiver_options().auto_accept(false)).get();

      std::cout << "test done" << std::endl;
   }
   catch (std::exception& e) {
      std::cerr << "exception in main: " << e.what() << std::endl;
   }
   catch (...) {
      std::cerr << "unknown error" << std::endl;
   }
}

TEST(main, errorHandlingAtStart)
{
   try {
      proton::Container().createConnection().open("//127.0.0.1:5673", proton::connection_options()).get();
   }
   catch (std::exception& e) {
      std::cerr << "Expected exception: " << e.what() << std::endl;
   }
}

TEST(main, DISABLED_errorHandlingAtReceive)
{
   try {
      Broker brk("//127.0.0.1:5672", "examples");
      //should simulate an error when send in broker
      proton::Container cont;
      proton::Connection conn = cont.createConnection();
      conn.open("//127.0.0.1:5672", proton::connection_options().idle_timeout(proton::duration(20))).get();

      proton::Session sess = conn.createSession();
      sess.open(proton::session_options()).get();

      proton::Receiver rec = sess.createReceiver();
      rec.open("examples", proton::receiver_options().auto_accept(false)).get();

      try {
         std::future<proton::Receiver::Delivery> awaitrec1 = rec.receive();
         awaitrec1.get();
      }
      catch (std::exception& e) {
         std::cerr << "Expected exception: " << e.what() << std::endl;
      }

      try {
         proton::Session sess = conn.createSession();
      }
      catch (std::exception& e) {
         std::cerr << "Expected exception: " << e.what() << std::endl;
      }
   }
   catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
   }
}


/* add tests:
- if open called many times
- if destination not created
*/
