#ifndef PROTON_IMPERATIVE_CLOSEREGISTRY_HPP
#define PROTON_IMPERATIVE_CLOSEREGISTRY_HPP

#include <proton/imperative/config.hpp>

#include <string>
#include <list>
#include <functional>

namespace proton {

class PROTON_IMPERATIVE_API CloseRegistry
{
public:
   void registerCallbacks(std::function<void(const std::string&)>* errorFn, std::function<void()>* closeFn);
   void unregisterCallbacks(std::function<void(const std::string&)>* errorFn, std::function<void()>* closeFn);
   void notifyError(const std::string& str);
   void notifyClose();

   CloseRegistry() = default;
   CloseRegistry(CloseRegistry&& other) = default;
   CloseRegistry& operator=(CloseRegistry&& other) = default;
   CloseRegistry(const CloseRegistry&) = delete;
   CloseRegistry& operator=(const CloseRegistry&) = delete;

private:
   std::list<std::function<void(const std::string&)>*> m_onErrorCallbacks;
   std::list<std::function<void()>*> m_onCloseCallbacks;
};

}

#endif
