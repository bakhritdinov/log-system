cmake_minimum_required(VERSION 3.15)
project(LogSystem C)

set(CMAKE_C_STANDARD 11)

# ==============================================================================
# FetchContent (uthash & unity)
# ==============================================================================
include(FetchContent)

FetchContent_Declare(
        uthash
        GIT_REPOSITORY https://github.com/troydhanson/uthash.git
        GIT_TAG        master
)

FetchContent_Declare(
        unity
        GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
        GIT_TAG        v2.5.2
)

FetchContent_MakeAvailable(uthash unity)

# ==============================================================================
# ZeroMQ
# ==============================================================================
if (WIN32)
    find_package(ZeroMQ CONFIG REQUIRED)
    set(ZMQ_TARGET libzmq)
else ()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ZMQ REQUIRED IMPORTED_TARGET libzmq)
    set(ZMQ_TARGET PkgConfig::ZMQ)
endif ()

# ==============================================================================
# Executable files
# ==============================================================================
add_executable(log_agent src/log_agent.c)
add_executable(log_collector src/log_collector.c)

target_link_libraries(log_agent PRIVATE ${ZMQ_TARGET})
target_include_directories(log_agent PRIVATE src ${uthash_SOURCE_DIR}/src)

target_link_libraries(log_collector PRIVATE ${ZMQ_TARGET})
target_include_directories(log_collector PRIVATE src ${uthash_SOURCE_DIR}/src)

# ==============================================================================
# Windows / Linux
# ==============================================================================
if (WIN32)
    target_link_libraries(log_agent PRIVATE Ws2_32 Iphlpapi)
    target_link_libraries(log_collector PRIVATE Ws2_32 Iphlpapi)
    target_compile_definitions(log_agent PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(log_collector PRIVATE _CRT_SECURE_NO_WARNINGS)
endif ()

if (UNIX AND NOT APPLE)
    target_link_libraries(log_agent PRIVATE rt)
endif ()

# ==============================================================================
# CTest
# ==============================================================================
enable_testing()

add_executable(run_tests
        tests/test_main.c
        tests/test_agent.c
        tests/test_collector.c
        src/log_agent.c
        src/log_collector.c
)

target_compile_definitions(run_tests PRIVATE TESTING)

target_link_libraries(run_tests PRIVATE unity ${ZMQ_TARGET})
target_include_directories(run_tests PRIVATE src ${uthash_SOURCE_DIR}/src)

add_test(NAME AllUnitTests COMMAND run_tests)