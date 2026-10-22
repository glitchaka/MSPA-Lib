cmake_minimum_required(VERSION 3.20)

project(MSPA_Lib LANGUAGES CXX)

option(MSPA_BUILD_TESTS "Build MSPA tests" ON)

add_library(mspa STATIC
    src/base/rng.cpp
    src/base/time.cpp
    src/semantics/registry.cpp
    src/psychology/engine.cpp
    src/psychology/beliefs.cpp
    src/psychology/appraisal.cpp
    src/psychology/emotions.cpp
    src/psychology/goals.cpp
    src/psychology/memory.cpp
    src/psychology/relationships.cpp
    src/psychology/learning.cpp
    src/decision/decision.cpp
    src/decision/hysteresis.cpp
    src/runtime/agent_store.cpp
    src/runtime/runtime.cpp
    src/persistence/state_transfer.cpp
    src/validation/limits.cpp
    src/diagnostics/trace.cpp
)

add_library(MSPA::mspa ALIAS mspa)
add_library(MSPA::base ALIAS mspa)

target_compile_features(mspa PUBLIC cxx_std_17)
target_include_directories(mspa
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if(MSVC)
    target_compile_options(mspa PRIVATE /W4 /permissive- /fp:strict)
else()
    target_compile_options(mspa PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)
endif()

if(MSPA_BUILD_TESTS)
    enable_testing()

    add_executable(mspa_base_tests tests/base_tests.cpp)
    target_compile_features(mspa_base_tests PRIVATE cxx_std_17)
    target_link_libraries(mspa_base_tests PRIVATE MSPA::mspa)

    add_executable(mspa_runtime_tests tests/runtime_tests.cpp)
    target_compile_features(mspa_runtime_tests PRIVATE cxx_std_17)
    target_link_libraries(mspa_runtime_tests PRIVATE MSPA::mspa)

    if(MSVC)
        target_compile_options(mspa_base_tests PRIVATE /W4 /permissive- /fp:strict)
        target_compile_options(mspa_runtime_tests PRIVATE /W4 /permissive- /fp:strict)
    else()
        target_compile_options(mspa_base_tests PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)
        target_compile_options(mspa_runtime_tests PRIVATE -Wall -Wextra -Wpedantic -ffp-contract=off)
    endif()

    add_test(NAME mspa_base_tests COMMAND mspa_base_tests)
    add_test(NAME mspa_runtime_tests COMMAND mspa_runtime_tests)
endif()
