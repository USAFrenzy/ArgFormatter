cmake_minimum_required(VERSION 3.14)

project(ArgFormatter)

option(
    BUILD_SANDBOX
    "Build The Sandbox Project Alongside The Library"
    OFF
)

option(
    BUILD_TESTS
    "Build Test Suite Alongside The Library"
    OFF
)

option(
    BUILD_ALL
    "Build Benchmarking, Testing, Demo, and Sandbox Projects Alongside The Library"
    ON
)


if (BUILD_SANDBOX)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sandbox ${CMAKE_CURRENT_BINARY_DIR}/sandbox)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Sandbox_Environment)
endif ()

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests ${CMAKE_CURRENT_BINARY_DIR}/tests)
endif ()

if (BUILD_ALL)
    enable_testing()
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests ${CMAKE_CURRENT_BINARY_DIR}/tests)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sandbox ${CMAKE_CURRENT_BINARY_DIR}/sandbox)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Sandbox_Environment)
endif ()