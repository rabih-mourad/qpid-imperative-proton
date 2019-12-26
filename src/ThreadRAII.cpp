#include <proton/imperative/ThreadRAII.hpp>

using namespace proton;

ThreadRAII::ThreadRAII(std::thread&& thread)
   : m_thread(std::move(thread))
{
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
