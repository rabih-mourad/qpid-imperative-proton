#include <proton/imperative/PromiseWithActiveFlag.hpp>

using namespace proton;


void proton::PromiseWithActiveFlag::activate()
{
   m_active = true;
}

std::future<void> PromiseWithActiveFlag::get_future()
{
   return m_promise.get_future();
}

void PromiseWithActiveFlag::set_value()
{
   m_active = false;
   m_promise.set_value();
}

void PromiseWithActiveFlag::set_exception(std::exception_ptr e_ptr)
{
   m_active = false;
   m_promise.set_exception(e_ptr);
}

bool PromiseWithActiveFlag::isActive()
{
   return m_active;
}
