#ifndef PROTON_IMPERATIVE_THREADRAII_HPP
#define PROTON_IMPERATIVE_THREADRAII_HPP

#include <thread>

class ThreadRAII
{
public:
   ThreadRAII() = default;
   ThreadRAII(std::thread&& thread);

   std::thread& get();

   ThreadRAII(ThreadRAII&& thread) = default;
   ThreadRAII& operator=(ThreadRAII&& thread) = default;
   ThreadRAII(const ThreadRAII& other) = delete;
   ThreadRAII& operator=(const ThreadRAII& other) = delete;

   ~ThreadRAII();

private:
   std::thread m_thread;
};

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


#endif
