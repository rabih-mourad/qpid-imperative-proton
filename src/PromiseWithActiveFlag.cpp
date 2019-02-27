#include <proton/imperative/PromiseWithActiveFlag.hpp>

using namespace proton;

PromiseWithActiveFlag::PromiseWithActiveFlag()
   :m_active(false)
{}

PromiseWithActiveFlag::PromiseWithActiveFlag(PromiseWithActiveFlag&& p)
   :m_promise(std::move(p.m_promise)),
   m_active(p.m_active)
{}

std::future<void> PromiseWithActiveFlag::get_future()
{
   m_active = true;
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
