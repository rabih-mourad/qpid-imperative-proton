#ifndef PROTON_IMPERATIVE_PNOBJECTLIFEMANAGER_HPP
#define PROTON_IMPERATIVE_PNOBJECTLIFEMANAGER_HPP

#include <proton/imperative/config.hpp>
#include <proton/imperative/PromiseWithActiveFlag.hpp>
#include <proton/imperative/CloseRegistry.hpp>

#include <proton/error_condition.hpp>

namespace proton {

class PROTON_IMPERATIVE_API PnObjectLifeManager
{
public:
   PnObjectLifeManager(CloseRegistry* parentCloseRegistry, std::function<void(const std::string&)> releasePnObjects);

   // Methods called on proton handlers side
   void handlePnError(std::string err);
   void handlePnClose();
   void handlePnOpen();

   // Methods called on user's thread side
   void checkForExceptions();
   std::future<void> getOpenFuture();
   std::future<void> close();
   // For simplicity, we will allow only one close. It can be changed afterwards.
   bool hasBeenClosed();
   bool isInError();

   CloseRegistry* getCloseRegistry();

   PnObjectLifeManager() = default;
   PnObjectLifeManager(PnObjectLifeManager&& other) = default;
   PnObjectLifeManager& operator=(PnObjectLifeManager&& other) = delete;
   PnObjectLifeManager(const PnObjectLifeManager&) = delete;
   PnObjectLifeManager& operator=(const PnObjectLifeManager&) = delete;

   const std::string errorParentClosed;
   std::mutex m_mutex;

private:
   PromiseWithActiveFlag m_onOpenPromise;
   PromiseWithActiveFlag m_onClosePromise;
   // This flag is set to true only when the user or destructor wants to close the object
   bool m_hasBeenClosed = false;
   std::exception_ptr m_exception;
   std::string m_exceptionMsg;
   CloseRegistry m_closeRegistry;
   CloseRegistry* m_parentCloseRegistry;
   std::function<void(const std::string&)> m_errorFn;
   std::function<void()> m_closeFn;
   std::function<void(const std::string&)> m_releasePnObjects;
};

}

#endif
