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
   struct SessionHandler : public messaging_handler
   {
      SessionHandler() = default;
      SessionHandler(SessionHandler&& c);

      void on_session_open(session& sess) override;
      void on_session_close(session&) override;
      void on_session_error(session& sess) override;

      PnObjectLifeManager m_manager;
      session m_session;
   };

public:
   std::future<void> open(session_options sess_opts);
   void close();
   Sender createSender();
   Receiver createReceiver();

   Session(connection conn);
   Session(Session&& c);
   ~Session();

private:
   SessionHandler m_sessionHandler;
   connection m_connection;
};

}

#endif
