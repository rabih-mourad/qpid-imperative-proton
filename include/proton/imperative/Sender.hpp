#ifndef PROTON_IMPERATIVE_SENDER_HPP
#define PROTON_IMPERATIVE_SENDER_HPP

#include <proton/message.hpp>
#include <proton/sender.hpp>
#include <proton/sender_options.hpp>
#include <proton/session.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/work_queue.hpp>
#include <proton/connection.hpp>
#include <proton/transport.hpp>

#include <proton/imperative/config.hpp>
#include <proton/imperative/PromiseWithActiveFlag.hpp>
#include <proton/imperative/PnObjectLifeManager.hpp>

#include <map>

namespace proton {

class SenderHandler;

class PROTON_IMPERATIVE_API Sender
{
   struct SenderHandler : public messaging_handler
   {
      SenderHandler() = default;
      SenderHandler(SenderHandler&& c);

      std::future<void> send(const message& mess);

      void on_sender_open(sender& sen) override;
      void on_sender_close(sender&) override;
      void on_sender_error(sender& sen) override;
      void on_tracker_accept(tracker &track) override;
      void on_tracker_reject(tracker &) override;
      void on_tracker_release(tracker &) override;

      std::promise<void> removeTrackerFromMapIfFoundElseThow(tracker &track);

      PnObjectLifeManager m_manager;
      sender m_sender;
      std::map<tracker, std::promise<void>> m_sentRequests;
   };

public:
   std::future<void> open(const std::string& address, sender_options send_opts);
   void close();
   std::future<void> send(const message& mess);

   Sender(session s);
   Sender(Sender&& c);
   ~Sender();

private:
   SenderHandler m_senderHandler;
   session m_session;
};

}

#endif
