cmake_minimum_required(VERSION 3.25)
project(jaql VERSION 0.1.0 LANGUAGES CXX)

option(JAQL_BUILD_TESTS "Build C++/Python tests" ON)
option(JAQL_BUILD_PYTHON "Build Python bindings" ON)
option(JAQL_BUILD_BENCHMARKS "Build benchmarks" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

add_library(jaql_core
    src/date.cpp
    src/day_count.cpp
    src/calendar.cpp
    src/instruments.cpp
    src/curves.cpp
    src/greeks.cpp
    src/monte_carlo.cpp
)
set_target_properties(jaql_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(jaql_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(JAQL_BUILD_PYTHON)
    find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v2.13.6
    )
    FetchContent_MakeAvailable(pybind11)

    pybind11_add_module(jaql MODULE python/bindings.cpp)
    target_link_libraries(jaql PRIVATE jaql_core)
    install(TARGETS jaql LIBRARY DESTINATION .)
endif()

if(JAQL_BUILD_TESTS)
    include(CTest)
    enable_testing()

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(jaql_cpp_tests
        tests/cpp/test_date_and_day_count.cpp
        tests/cpp/test_instruments_and_curves.cpp
        tests/cpp/test_greeks.cpp
    )
    target_link_libraries(jaql_cpp_tests PRIVATE jaql_core Catch2::Catch2WithMain)

    include(Catch)
    catch_discover_tests(jaql_cpp_tests)
endif()

if(JAQL_BUILD_BENCHMARKS)
    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.1
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)

    add_executable(jaql_benchmarks benchmarks/black_scholes_benchmark.cpp)
    target_link_libraries(jaql_benchmarks PRIVATE jaql_core benchmark::benchmark)
endif()
