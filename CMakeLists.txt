cmake_minimum_required(VERSION 3.20)

project(MSPA_Lib LANGUAGES CXX)

option(MSPA_BUILD_TESTS "Build MSPA tests" ON)

add_library(mspa_base STATIC
    src/base/rng.cpp
)

add_library(MSPA::base ALIAS mspa_base)

target_compile_features(mspa_base PUBLIC cxx_std_17)
target_include_directories(mspa_base
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if(MSVC)
    target_compile_options(mspa_base PRIVATE /W4 /permissive- /fp:strict)
else()
    target_compile_options(mspa_base PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)
endif()

if(MSPA_BUILD_TESTS)
    enable_testing()

    add_executable(mspa_base_tests
        tests/base_tests.cpp
    )

    target_compile_features(mspa_base_tests PRIVATE cxx_std_17)
    target_link_libraries(mspa_base_tests PRIVATE MSPA::base)

    if(MSVC)
        target_compile_options(mspa_base_tests PRIVATE /W4 /permissive- /fp:strict)
    else()
        target_compile_options(mspa_base_tests PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)
    endif()

    add_test(NAME mspa_base_tests COMMAND mspa_base_tests)
endif()
