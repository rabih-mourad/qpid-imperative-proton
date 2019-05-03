#ifndef PROTON_IMPERATIVE_PNOBJECTLIFEMANAGER_HPP
#define PROTON_IMPERATIVE_PNOBJECTLIFEMANAGER_HPP

#include <proton/imperative/config.hpp>
#include <proton/imperative/PromiseWithActiveFlag.hpp>

#include <proton/error_condition.hpp>

namespace proton {

class PROTON_IMPERATIVE_API PnObjectLifeManager
{
public:
   PnObjectLifeManager();
   PnObjectLifeManager(PnObjectLifeManager&& c);

   // methods called on proton handlers side
   void handlePnError(std::string err);
   void handlePnClose(std::function<void()> releasePnObjects);
   void handlePnOpen();

   // methods called on user's thread side
   void checkForExceptions();
   std::future<void> getOpenFuture();
   std::future<void> close();
   //For simplicity, we will allow only one close. It can be changed afterwards.
   bool hasBeenClosedOrInError();

private:
   PromiseWithActiveFlag m_onOpenPromise;
   PromiseWithActiveFlag m_onClosePromise;
   bool m_hasBeenClosed = false;
   std::exception_ptr m_exception;
};

}

#endif
