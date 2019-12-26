#ifndef PROTON_IMPERATIVE_THREADRAII_HPP
#define PROTON_IMPERATIVE_THREADRAII_HPP

#include <proton/imperative/config.hpp>

#include <thread>

namespace proton {

class PROTON_IMPERATIVE_API ThreadRAII
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

}

#endif
