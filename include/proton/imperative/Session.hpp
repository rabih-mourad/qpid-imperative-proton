#ifndef PROTON_IMPERATIVE_SESSION_HPP
#define PROTON_IMPERATIVE_SESSION_HPP

#include <proton/session_options.hpp>
#include <proton/work_queue.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/session.hpp>
#include <proton/connection.hpp>

#include <proton/imperative/config.hpp>
#include <proton/imperative/Sender.hpp>
#include <proton/imperative/Receiver.hpp>
#include <proton/imperative/PromiseWithActiveFlag.hpp>
#include <proton/imperative/PnObjectLifeManager.hpp>

namespace proton {

class PROTON_IMPERATIVE_API Session
{
public:
   std::future<void> getOpenFuture();
   void close();
   Sender openSender(const std::string& address, sender_options send_opts);
   Receiver openReceiver(const std::string& address, receiver_options rec_opts);

   Session(Session&& c);
   ~Session();
   Session(const Session& other) = delete;
   Session& operator=(const Session& other) = delete;
   Session& operator=(Session&& other) = delete;

private:
   struct SessionHandler : public messaging_handler
   {
      SessionHandler(work_queue* w);
      SessionHandler(SessionHandler&& c);

      void on_session_open(session& sess) override;
      void on_session_close(session&) override;
      void on_session_error(session& sess) override;
      void releasePnMemberObjects();

      PnObjectLifeManager m_manager;
      session m_session;
      //to be able to chain commands without waiting for session to be initialized;
      work_queue* m_work;
   };

   Session(connection& con, work_queue* work, session_options sess_opts);
   friend class Connection;

   std::unique_ptr<SessionHandler> m_sessionHandler;
};

}

#endif
