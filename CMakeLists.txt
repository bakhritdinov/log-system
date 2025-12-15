cmake_minimum_required(VERSION 3.15)
project(LogSystem C)

set(CMAKE_C_STANDARD 11)

# =============================
# ZeroMQ
# =============================

if (WIN32)
    find_package(ZeroMQ CONFIG REQUIRED)
    set(ZMQ_TARGET libzmq)
else ()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ZMQ REQUIRED IMPORTED_TARGET libzmq)
    set(ZMQ_TARGET PkgConfig::ZMQ)
endif ()

# =============================
# Targets
# =============================

add_executable(log_agent src/log_agent.c)
add_executable(log_collector src/log_collector.c)

target_link_libraries(log_agent PRIVATE ${ZMQ_TARGET})
target_link_libraries(log_collector PRIVATE ${ZMQ_TARGET})

# =============================
# Windows system libs
# =============================

if (WIN32)
    target_link_libraries(log_agent PRIVATE Ws2_32 Iphlpapi)
    target_link_libraries(log_collector PRIVATE Ws2_32 Iphlpapi)
endif ()
