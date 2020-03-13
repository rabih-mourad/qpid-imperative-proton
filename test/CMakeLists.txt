set(TARGET qpid-imperative-proton-test)

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
find_package(Gtest REQUIRED)

target_include_directories(${TARGET} PRIVATE
    ${GTEST_INCLUDE_DIR}
    ${QPID_PROTON_INCLUDE_DIR}
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/test")

target_link_libraries(${TARGET} qpid-imperative-proton ${GTEST_BOTH_LIBRARIES} ${QPID_PROTON_CPP_LIBRARIES})

ADD_TEST("${TARGET}-suite" ${TARGET})

