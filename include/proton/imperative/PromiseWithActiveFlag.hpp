#ifndef PROTON_IMPERATIVE_PROMISEWITHACTIVEFLAG_HPP
#define PROTON_IMPERATIVE_PROMISEWITHACTIVEFLAG_HPP

#include <proton/imperative/config.hpp>

#include <future>
#include <atomic>

namespace proton {

class PROTON_IMPERATIVE_API PromiseWithActiveFlag
{
public:
   PromiseWithActiveFlag() = default;
   PromiseWithActiveFlag(PromiseWithActiveFlag&& p) = default;
   PromiseWithActiveFlag(const PromiseWithActiveFlag& other) = delete;
   PromiseWithActiveFlag& operator=(const PromiseWithActiveFlag& other) = delete;
   PromiseWithActiveFlag& operator=(PromiseWithActiveFlag&& other) = delete;

   void activate();
   std::future<void> get_future();
   void set_value();
   void set_exception(std::exception_ptr e_ptr);
   bool isActive();

private:
   std::promise<void> m_promise;
   std::atomic<bool> m_active;
};

}

#endif
