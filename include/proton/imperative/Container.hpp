#ifndef PROTON_IMPERATIVE_CONTAINER_HPP
#define PROTON_IMPERATIVE_CONTAINER_HPP

#include <proton/imperative/config.hpp>
#include <proton/imperative/Fwd.hpp>

#include <proton/fwd.hpp>

#include <memory>
#include <string>

namespace proton {

class Container
{
public:
   PROTON_IMPERATIVE_API Container();
   

   //This method is synchronous because there is no thread safe way in proton to get the connection
   PROTON_IMPERATIVE_API Connection openConnection(const std::string& url, connection_options conn_opts);

   PROTON_IMPERATIVE_API void close();

   PROTON_IMPERATIVE_API Container(const Container& other) = default;
   PROTON_IMPERATIVE_API Container& operator=(const Container& other) = default;
   PROTON_IMPERATIVE_API Container(Container&& c) = default;
   PROTON_IMPERATIVE_API Container& operator=(Container&& other) = default;
   PROTON_IMPERATIVE_API ~Container() = default;

private:
   class Impl;
   std::shared_ptr<Impl> m_impl;
};

}

#endif
