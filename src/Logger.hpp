#ifndef PROTON_IMPERATIVE_LOGGER_HPP
#define PROTON_IMPERATIVE_LOGGER_HPP

#include <iostream>

namespace {

class Log {
public:
   template <typename T>
   const Log& operator<< (T&& log) const
   {
   #ifdef ENABLE_DEBUG_MODE
      std::cout << std::forward<T>(log);
   #endif
      return *this;
   }
};


const Log& operator<< (const Log& log, std::ostream& (stream) (std::ostream&)) 
{
#ifdef ENABLE_DEBUG_MODE
   std::cout << stream;
#endif
   return log;
}

}

#endif
