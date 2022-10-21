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
    "Build Tests and Sandbox Projects Alongside The Library"
    OFF
)

option(
    BUILD_COMPILED_LIB
    "Build The ArgFormatter Library As A Compiled Library To Link Against"
    OFF
)

if (BUILD_COMPILED_LIB)
    add_compile_definitions(COMPILED_LIBRARY)
    add_subdirectory(src)
endif ()

include(${CMAKE_SOURCE_DIR}/cmake/CPM.cmake)
if (BUILD_COMPILED_LIB)
    cpmaddpackage(
        NAME
        utfutils
        GITHUB_REPOSITORY
        USAFrenzy/UTF-Utils
        VERSION
        1.0.0
        OPTIONS
        "BUILD_COMPILED_LIBRARY On"
    )
else ()
    cpmaddpackage(
        NAME
        utfutils
        GITHUB_REPOSITORY
        USAFrenzy/UTF-Utils
        VERSION
        1.0.0
    )
endif ()

if (utfutils_ADDED)
    file(
        GLOB
        UTFUTILS_SOURCES
        ${utfutils_SOURCE_DIR}/src/*.cpp
    )
    add_library(utfutils STATIC ${UTFUTILS_SOURCES})
    set(STANDARD 20)
    set_target_properties(utfutils PROPERTIES CXX_STANDARD ${STANDARD})
    target_include_directories(utfutils PUBLIC $<BUILD_INTERFACE:${utfutils_SOURCE_DIR}/include>)
    if (BUILD_COMPILED_LIB)
        target_include_directories(
            ArgFormatter_Lib PUBLIC $<BUILD_INTERFACE:${utfutils_SOURCE_DIR}/include>
        )
        target_link_libraries(ArgFormatter_Lib utfutils)
    else ()
        file(COPY ${utfutils_SOURCE_DIR}/include/UTF-Utils
             DESTINATION ${CMAKE_SOURCE_DIR}/include/ArgFormatter/dependencies
        )
    endif ()
endif ()

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
