cmake_minimum_required(VERSION 3.2.3)

project(qpid-imperative-proton-test)
set(TARGET qpid-imperative-proton-test)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-cfg")
include(CompilerFlags)

set (sources
    Broker.hpp
    Constants.hpp
	ContainerTest.cpp
	ConnectionTest.cpp
	SessionTest.cpp
	SenderTest.cpp
	ReceiverTest.cpp
	DeliveryTest.cpp)
add_executable(${TARGET} ${sources})

find_package(QpidProton REQUIRED)
target_include_directories(${TARGET} PRIVATE ${QPID_PROTON_INCLUDE_DIR})
find_package(Gtest REQUIRED)
target_include_directories(${TARGET} PRIVATE ${GTEST_INCLUDE_DIR})

target_include_directories(${TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_include_directories(${TARGET} PRIVATE "${CMAKE_SOURCE_DIR}/test")
target_link_libraries(${TARGET} qpid-imperative-proton ${GTEST_BOTH_LIBRARIES} ${QPID_PROTON_CPP_LIBRARIES})

enable_testing()
ADD_TEST(NAME ${TARGET} COMMAND ${TARGET})
