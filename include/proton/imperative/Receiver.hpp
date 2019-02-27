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

namespace proton {

class PROTON_IMPERATIVE_API Receiver
{
public:
   struct Delivery
   {
      Delivery() = default;
      Delivery(delivery d, message m)
         :m_delivery(d), pn_message(m)
      {}
      
      void accept()
      {
         m_delivery.work_queue().add([=]() {m_delivery.accept(); });
      }

      void reject()
      {
         m_delivery.work_queue().add([=]() {m_delivery.reject(); });
      }

      void release()
      {
         m_delivery.work_queue().add([=]() {m_delivery.release(); });
      }

      message pn_message;
   private:
      delivery m_delivery;
   };

private:
   struct ReceiverHandler : public messaging_handler
   {
      ReceiverHandler() = default;
      ReceiverHandler(ReceiverHandler&& c);

      std::future<Delivery> receive();

      void on_receiver_open(receiver& sen) override;
      void on_receiver_close(receiver&) override;
      void on_receiver_error(receiver& sen) override;
      void on_message(delivery&, message&) override;
      //void on_receiver_drain_finish(receiver&) override;

      PnObjectLifeManager m_manager;
      receiver m_receiver;
      std::queue<std::promise<Receiver::Delivery>> m_promisesQueue;
   };

public:
   std::future<void> open(const std::string& address, receiver_options send_opts);
   void close();
   std::future<Delivery> receive();

   Receiver(session sess);
   Receiver(Receiver&& c);
   ~Receiver();

private:
   ReceiverHandler m_receiverHandler;
   session m_session;
};

}

#endif
