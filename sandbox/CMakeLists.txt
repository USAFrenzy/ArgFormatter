set(PROJECT_NAME Sandbox_Environment)

message("-- Building ${PROJECT_NAME}")

set(SANDBOX_SOURCE_FILES "sandbox.cpp")
add_executable(${PROJECT_NAME} ${SANDBOX_SOURCE_FILES})

set(STANDARD 20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD ${STANDARD})

target_include_directories(${PROJECT_NAME} PUBLIC ${ARGFMT_INCLUDE_DIR})

if (BUILD_COMPILED_LIB)
    target_link_libraries(
        ${PROJECT_NAME}
        LINK_PUBLIC
        ArgFormatter_Lib
    )
endif ()
