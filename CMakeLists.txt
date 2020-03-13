cmake_minimum_required(VERSION 3.2.3)

project(qpid-imperative-proton)
set(TARGET qpid-imperative-proton)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-cfg")
include(CompilerFlags)

set (headers
    include/proton/imperative/config.hpp
    include/proton/imperative/Fwd.hpp
    include/proton/imperative/Connection.hpp
    include/proton/imperative/Container.hpp
    include/proton/imperative/Session.hpp
    include/proton/imperative/Sender.hpp
    include/proton/imperative/Receiver.hpp
    include/proton/imperative/Delivery.hpp
    include/proton/imperative/PromiseWithActiveFlag.hpp
    include/proton/imperative/PnObjectLifeManager.hpp
    include/proton/imperative/CloseRegistry.hpp)

set (sources
    src/Container.cpp
    src/Connection.cpp
    src/Logger.hpp
    src/Session.cpp
    src/Sender.cpp
    src/Receiver.cpp
    src/Delivery.cpp
    src/ThreadRAII.hpp
    src/PromiseWithActiveFlag.cpp
    src/PnObjectLifeManager.cpp
    src/CloseRegistry.cpp)
add_library(${TARGET} SHARED ${headers} ${sources})

find_package(QpidProton REQUIRED)

add_definitions(-DPROTON_IMPERATIVE_EXPORTS)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -DENABLE_DEBUG_MODE -O0")

target_include_directories(${TARGET} 
   PRIVATE ${QPID_PROTON_INCLUDE_DIR} "${CMAKE_SOURCE_DIR}/src"
   PUBLIC "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(${TARGET} ${GTEST_BOTH_LIBRARIES} ${QPID_PROTON_CPP_LIBRARIES})

add_subdirectory(test)
