#include <proton/imperative/CloseRegistry.hpp>

using namespace proton;

void CloseRegistry::registerCallbacks(std::function<void(const std::string&)>* errorFn, std::function<void()>* closeFn)
{
   m_onErrorCallbacks.emplace_back(errorFn);
   m_onCloseCallbacks.emplace_back(closeFn);
}

void CloseRegistry::unregisterCallbacks(std::function<void(const std::string&)>* errorFn, std::function<void()>* closeFn)
{
   m_onErrorCallbacks.remove(errorFn);
   m_onCloseCallbacks.remove(closeFn);
}

void CloseRegistry::notifyError(const std::string& str)
{
   for (auto it : m_onErrorCallbacks) {
      (*it)(str);
   }
}

void CloseRegistry::notifyClose()
{
   std::list<std::function<void()>*>::iterator it = m_onCloseCallbacks.begin();
   while (it != m_onCloseCallbacks.end())
   {
      std::list<std::function<void()>*>::iterator current = it;
      ++it;
      (**current)();
   }
}
