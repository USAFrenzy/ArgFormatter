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

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)

# Set this here so that the lib object branch can link against this path as well
set(UFT_UTILS_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/_deps/utfutils-src/include)

if (BUILD_COMPILED_LIB)
    add_compile_definitions(COMPILED_LIBRARY)
    # if we're building a lib object and this has been included via an external project, if the
    # dependencies folder is present, get rid of it for compatibility's sake
    if (EXISTS ${CMAKE_BINARY_DIR}/argfmt-src/include/ArgFormatter/dependencies)
        file(REMOVE_RECURSE ${CMAKE_BINARY_DIR}/argfmt-src/include/ArgFormatter/dependencies)
    endif ()
    # Similar reasoning to the above; if we're builing a lib object as a master project and the
    # dependencies folder is present, get rid of it for compatibility's sake
    if (EXISTS ${CMAKE_SOURCE_DIR}/include/ArgFormatter/dependencies)
        file(REMOVE_RECURSE ${CMAKE_SOURCE_DIR}/include/ArgFormatter/dependencies)
    endif ()
    cpmaddpackage(
        NAME
        utfutils
        GITHUB_REPOSITORY
        USAFrenzy/UTF-Utils
        VERSION
        1.0.1
        OPTIONS
        "BUILD_COMPILED_LIBRARY ON"
    )
    add_subdirectory(src)
elseif (NOT BUILD_COMPILED_LIB AND COMMAND COMPILED_LIBRARY)
    # If the 'COMPILED_MACRO' is defined, then that means at some point, a compiled version of this
    # library was built or that a user felt like defining this macro for some reason. Either way, to
    # maintain compatibility and known behavior, invalidate the compiled version by copying over the
    # dependency files and removing the macro define unless the dependency project was built
    # separately in an external project. In the case that UTF_Utils was built separately in an
    # external project and the includes path is present, point the header-only include path to the
    # already grabbed dependency project and keep the macro define for correct file inclusion and to
    # avoid multiple symbols being produced.
    if (EXISTS ${UFT_UTILS_INCLUDE_DIRS})
        include_directories(${UFT_UTILS_INCLUDE_DIRS})
    else ()
        remove_definitions(COMPILED_LIBRARY)
        cpmaddpackage(
            NAME
            utfutils
            GITHUB_REPOSITORY
            USAFrenzy/UTF-Utils
            VERSION
            1.0.1
            "BUILD_COMPILED_LIBRARY OFF"
        )
        file(COPY ${utfutils_SOURCE_DIR}/include/UTF-Utils
             DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include/ArgFormatter/dependencies
        )
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/ArgFormatter/dependencies)
    endif ()
else ()
    # If not building a compiled library and the COMPILED_LIBRARY macro isn't defined, just copy
    # over dependencies
    cpmaddpackage(
        NAME
        utfutils
        GITHUB_REPOSITORY
        USAFrenzy/UTF-Utils
        VERSION
        1.0.1
        "BUILD_COMPILED_LIBRARY OFF"
    )
    file(COPY ${utfutils_SOURCE_DIR}/include/UTF-Utils
         DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/include/ArgFormatter/dependencies
    )
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/ArgFormatter/dependencies)
endif ()

set(ARGFMT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

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
