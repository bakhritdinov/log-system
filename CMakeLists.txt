cmake_minimum_required(VERSION 3.15)
project(LogSystem C)

set(CMAKE_C_STANDARD 11)

find_package(ZeroMQ CONFIG REQUIRED)

add_executable(log_agent src/log_agent.c)
target_link_libraries(log_agent PRIVATE libzmq)

add_executable(log_collector src/log_collector.c)
target_link_libraries(log_collector PRIVATE libzmq)

if (WIN32)
    target_link_libraries(log_agent PRIVATE Ws2_32 Iphlpapi)
    target_link_libraries(log_collector PRIVATE Ws2_32 Iphlpapi)
endif()
