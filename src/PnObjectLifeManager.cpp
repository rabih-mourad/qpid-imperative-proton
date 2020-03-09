#include <proton/imperative/PnObjectLifeManager.hpp>

#include <Logger.hpp>

using namespace proton;

namespace {
   Log debug_log;
}

PnObjectLifeManager::PnObjectLifeManager(CloseRegistry* parentCloseRegistry, std::function<void(const std::string&)> releasePnObjects)
   : m_parentCloseRegistry(parentCloseRegistry), m_releasePnObjects(releasePnObjects), errorParentClosed("Parent object was closed")
{
   m_onOpenPromise.activate();
   if (m_parentCloseRegistry != nullptr) {
      m_errorFn = [&](std::string str) { this->handlePnError(str); };
      m_closeFn = [&] { this->handlePnClose(); };
      m_parentCloseRegistry->registerCallbacks(&m_errorFn, &m_closeFn);
   }
}

void PnObjectLifeManager::handlePnError(std::string errorMsg)
{
   debug_log << "---handlePnError start" << std::endl;
   m_exception = std::make_exception_ptr(std::runtime_error(errorMsg));
   m_exceptionMsg = errorMsg;
   m_closeRegistry.notifyError(errorMsg);
   m_onClosePromise.activate();

   if (m_onOpenPromise.isActive())
   {
      m_onOpenPromise.set_exception(m_exception);
   }
   debug_log << "---handlePnError finish" << std::endl;
}

void PnObjectLifeManager::handlePnClose()
{
   debug_log << "---handlePnClose start" << std::endl;
   if (!m_exception)
   {
      // If the eventloop called close on a handler without the user requesting, nor the destructor called nor on_x_error is called
      if (m_hasBeenClosed == false)
      {
         m_exception = std::make_exception_ptr(std::runtime_error(errorParentClosed+" without explicit action"));
      }

      // If a graceful close was called on the object
      debug_log << "---handlePnClose no except" << std::endl;
      m_closeRegistry.notifyError(errorParentClosed);
      m_closeRegistry.notifyClose();
      m_releasePnObjects("Object was closed");
   }
   // If a close was called on the object after an on_x_error
   else
   {
      debug_log << "---handlePnClose in except" << std::endl;
      m_closeRegistry.notifyClose();
      m_releasePnObjects(m_exceptionMsg);
      debug_log << "---handlePnClose after release" << std::endl;
   }

   if (m_parentCloseRegistry != nullptr)
   {
      // TO DO check how to unregister callbacks without being in the loop
      m_parentCloseRegistry->unregisterCallbacks(&m_errorFn, &m_closeFn);
   }

   if (m_onOpenPromise.isActive())
   {
      m_onOpenPromise.set_exception(m_exception);
   }
   if (m_onClosePromise.isActive())
   {
      if(m_exception)
         m_onClosePromise.set_exception(m_exception);
      else
         m_onClosePromise.set_value();
   }
}

void PnObjectLifeManager::checkForExceptions()
{
   if (m_exception)
   {
      std::rethrow_exception(m_exception);
   }
   if (m_hasBeenClosed)
   {
      throw std::runtime_error("Can't execute the action because the object was closed");
   }
}

std::future<void> PnObjectLifeManager::getOpenFuture()
{
   return m_onOpenPromise.get_future();
}

void PnObjectLifeManager::handlePnOpen()
{
   m_onOpenPromise.set_value();
}

std::future<void> PnObjectLifeManager::close()
{
   m_hasBeenClosed = true;
   m_onClosePromise.activate();
   return m_onClosePromise.get_future();
}

bool PnObjectLifeManager::hasBeenClosed()
{
   return m_hasBeenClosed;
}

bool PnObjectLifeManager::isInError()
{
   return !!m_exception;
}

CloseRegistry* PnObjectLifeManager::getCloseRegistry()
{
   return &m_closeRegistry;
}
