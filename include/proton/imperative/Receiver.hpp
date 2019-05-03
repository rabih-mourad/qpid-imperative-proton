#ifndef PROTON_IMPERATIVE_RECEIVER_HPP
#define PROTON_IMPERATIVE_RECEIVER_HPP

#include <proton/message.hpp>
#include <proton/receiver.hpp>
#include <proton/session.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/work_queue.hpp>
#include <proton/delivery.hpp>
#include <proton/receiver_options.hpp>
#include <proton/connection.hpp>
#include <proton/transport.hpp>

#include <proton/imperative/config.hpp>
#include <proton/imperative/PromiseWithActiveFlag.hpp>
#include <proton/imperative/PnObjectLifeManager.hpp>

#include <queue>
#include <list>

namespace proton {

class Delivery;

class PROTON_IMPERATIVE_API Receiver
{
public:
   std::future<void> getOpenFuture();
   void close();
   std::future<Delivery> receive();

   Receiver(Receiver&& c);
   ~Receiver();
   Receiver(const Receiver& other) = delete;
   Receiver& operator=(const Receiver& other) = delete;
   Receiver& operator=(Receiver&& other) = delete;

private:
   struct ReceiverHandler : public messaging_handler
   {
      ReceiverHandler(work_queue* work);
      ReceiverHandler(ReceiverHandler&& c);

      std::future<Delivery> receive();

      void on_receiver_open(receiver& sen) override;
      void on_receiver_close(receiver&) override;
      void on_receiver_error(receiver& sen) override;
      void on_message(delivery&, message&) override;
      //To do:
      //void on_receiver_drain_finish(receiver&) override;

      void releasePnMemberObjects();

      PnObjectLifeManager m_manager;
      std::mutex m_queueLock;
      std::queue<std::promise<Delivery>> m_promisesQueue;
      std::mutex m_listLock;
      std::list<delivery> m_unclosedDeliveries;
      receiver m_receiver;
      //to be able to chain commands without waiting for session to be initialized;
      work_queue* m_work;
   };

   Receiver(work_queue* work, session& sess, const std::string& address, receiver_options rec_opts);
   friend class Session;

   std::unique_ptr<ReceiverHandler> m_receiverHandler;
};

}

#endif
