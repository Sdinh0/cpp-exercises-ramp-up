cmake_minimum_required(VERSION 3.20)
project(sensor_ramp_up LANGUAGES CXX)

include(CTest)
include(FetchContent)

option(SENSOR_ENABLE_WARNINGS "Enable useful compiler warnings" ON)

add_library(sensor_core
    src/sensor.cpp
)
target_include_directories(sensor_core PUBLIC include)
target_compile_features(sensor_core PUBLIC cxx_std_17)

if(SENSOR_ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(sensor_core PRIVATE /W4 /permissive-)
    else()
        target_compile_options(sensor_core PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(sensor_exercises
    tests/wire_format_tests.cpp
    tests/codec_boundaries_tests.cpp
    tests/stream_framing_tests.cpp
    tests/stream_recovery_tests.cpp
    tests/sample_window_tests.cpp
    tests/sensor_lifecycle_tests.cpp
)
target_link_libraries(sensor_exercises PRIVATE sensor_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(sensor_exercises)
