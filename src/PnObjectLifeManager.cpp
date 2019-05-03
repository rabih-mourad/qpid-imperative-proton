#include <proton/imperative/PnObjectLifeManager.hpp>

using namespace proton;

PnObjectLifeManager::PnObjectLifeManager()
   :m_onOpenPromise(true)
{}

PnObjectLifeManager::PnObjectLifeManager(PnObjectLifeManager&& c)
   : m_onOpenPromise(std::move(c.m_onOpenPromise)),
   m_onClosePromise(std::move(c.m_onClosePromise)),
   m_hasBeenClosed(c.m_hasBeenClosed),
   m_exception(std::move(c.m_exception))
{}

void PnObjectLifeManager::handlePnError(std::string errorMsg)
{
   m_exception = std::make_exception_ptr(std::runtime_error(errorMsg));
}

void PnObjectLifeManager::handlePnClose(std::function<void()> releasePnObjects)
{
   releasePnObjects();
   if (m_onOpenPromise.isActive())
   {
      m_onOpenPromise.set_exception(m_exception);
      return;
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
   return m_onClosePromise.get_future();
}

bool PnObjectLifeManager::hasBeenClosedOrInError()
{
   return m_hasBeenClosed || m_exception;
}
