#ifndef PROTON_IMPERATIVE_PROMISEWITHACTIVEFLAG_HPP
#define PROTON_IMPERATIVE_PROMISEWITHACTIVEFLAG_HPP

#include <proton/imperative/config.hpp>

#include <future>

namespace proton {

class PROTON_IMPERATIVE_API PromiseWithActiveFlag
{
public:
   PromiseWithActiveFlag();
   PromiseWithActiveFlag(PromiseWithActiveFlag&& p);

   std::future<void> get_future();
   void set_value();
   void set_exception(std::exception_ptr e_ptr);
   bool isActive();

private:
   std::promise<void> m_promise;
   bool m_active;
};

}

#endif
