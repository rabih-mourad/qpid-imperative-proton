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

   void handlePnError(std::string err);
   void checkForExceptions();
   std::future<void> open();
   void finishedOpenning();
   //For simplicity, we will allow only one open. It can be changed afterwards.
   bool canBeOpened();
   std::future<void> close();
   void finishedClosing();
   //For simplicity, we will allow only one close. It can be changed afterwards.
   bool canBeClosed();

private:
   PromiseWithActiveFlag m_onOpenPromise;
   PromiseWithActiveFlag m_onClosePromise;
   bool m_hasBeenOpened;
   bool m_hasBeenClosed;
   std::exception_ptr m_exception;
};

}

#endif
