#include <proton/imperative/ThreadRAII.hpp>

using namespace proton;

ThreadRAII::ThreadRAII(std::thread&& thread)
   : m_thread(std::move(thread))
{
}

ThreadRAII::ThreadRAII(ThreadRAII&& thread)
   : m_thread(std::move(thread.m_thread))
{
}

ThreadRAII& ThreadRAII::operator=(ThreadRAII&& thread)
{
   m_thread = std::move(thread.m_thread);
   return *this;
}

std::thread& ThreadRAII::get()
{
   return m_thread;
}

ThreadRAII::~ThreadRAII()
{
   if (m_thread.joinable())
   {
      m_thread.join();
   }
}
