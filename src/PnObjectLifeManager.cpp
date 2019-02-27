#include <proton/imperative/PnObjectLifeManager.hpp>

using namespace proton;

PnObjectLifeManager::PnObjectLifeManager()
   : m_hasBeenOpened(false), m_hasBeenClosed(false)
{}

PnObjectLifeManager::PnObjectLifeManager(PnObjectLifeManager&& c)
   : m_onOpenPromise(std::move(c.m_onOpenPromise)),
   m_onClosePromise(std::move(c.m_onClosePromise)),
   m_hasBeenOpened(c.m_hasBeenOpened),
   m_hasBeenClosed(c.m_hasBeenClosed),
   m_exception(std::move(c.m_exception))
{}

void PnObjectLifeManager::handlePnError(std::string errorMsg)
{
   m_exception = std::make_exception_ptr(std::runtime_error(errorMsg));
   if (m_onOpenPromise.isActive())
   {
      m_onOpenPromise.set_exception(m_exception);
      return;
   }
   if (m_onClosePromise.isActive())
   {
      m_onClosePromise.set_exception(m_exception);
      return;
   }
}

void PnObjectLifeManager::checkForExceptions()
{
   if (m_exception)
   {
      std::rethrow_exception(m_exception);
   }
}

std::future<void> PnObjectLifeManager::open()
{
   m_hasBeenOpened = true;
   return m_onOpenPromise.get_future();
}

void PnObjectLifeManager::finishedOpenning()
{
   m_onOpenPromise.set_value();
}

bool PnObjectLifeManager::canBeOpened()
{
   return !m_hasBeenOpened;
}

std::future<void> PnObjectLifeManager::close()
{
   m_hasBeenClosed = true;
   return m_onClosePromise.get_future();
}

void PnObjectLifeManager::finishedClosing()
{
   m_onClosePromise.set_value();
}

bool PnObjectLifeManager::canBeClosed()
{
   return !m_hasBeenClosed && m_hasBeenOpened && !m_exception;
}