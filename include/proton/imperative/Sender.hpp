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

class PROTON_IMPERATIVE_API Sender
{
public:
   std::future<void> getOpenFuture();
   void close();
   std::future<void> send(const message& mess);

   ~Sender();
   Sender(Sender&& c) = default;
   Sender(const Sender& other) = delete;
   Sender& operator=(const Sender& other) = delete;
   Sender& operator=(Sender&& other) = delete;

private:
   struct SenderHandler : public messaging_handler
   {
      SenderHandler(work_queue* work, CloseRegistry* sessionCloseRegistry);
      SenderHandler(SenderHandler&& c) = default;
      SenderHandler(const SenderHandler& other) = delete;
      SenderHandler& operator=(const SenderHandler& other) = delete;
      SenderHandler& operator=(SenderHandler&& other) = delete;

      std::future<void> send(const message& mess);

      void on_sender_open(sender& sen) override;
      void on_sender_close(sender&) override;
      void on_sender_error(sender& sen) override;
      void on_tracker_accept(tracker &track) override;
      void on_tracker_reject(tracker &) override;
      void on_tracker_release(tracker &) override;

      std::shared_ptr<std::promise<void>> removeTrackerFromMapIfFoundElseThow(tracker &track);
      void releasePnMemberObjects(const std::string&);

      PnObjectLifeManager m_manager;
      sender m_sender;
      //to be able to chain commands without waiting for session to be initialized;
      work_queue* m_work;
      std::map<tracker, std::shared_ptr<std::promise<void>>> m_sentRequests;
      std::mutex m_mapMutex;
   };

   Sender(work_queue* work, session& s, const std::string& address, sender_options send_opts, CloseRegistry* sessionCloseRegistry);
   friend class Session;

   std::unique_ptr<SenderHandler> m_senderHandler;
};

}

#endif
