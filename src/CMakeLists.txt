set(PROJECT_NAME ArgFormatter_Lib)
message("-- Building ${PROJECT_NAME}")

# Individual Groups Of Files
set(ARG_FMT_HEADERS
    "../include/ArgFormatter/ArgContainer.h"
    "../include/ArgFormatter/ArgContainerImpl.h"
    "../include/ArgFormatter/ArgFormatter.h"
    "../include/ArgFormatter/ArgFormatterImpl.h"
)

set(ARG_FMT_SOURCES "AF_Compiled.cpp")

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/../include/"
    PREFIX "Header Files"
    FILES ${ARG_FMT_HEADERS}
)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Source Files"
    FILES ${ARG_FMT_SOURCES}
)

set(ARG_FMT_ALL_FILES ${ARG_FMT_SOURCES} ${ARG_FMT_HEADERS})

add_library(${PROJECT_NAME} ${ARG_FMT_ALL_FILES})

target_include_directories(
    ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${UFT_UTILS_INCLUDE_DIRS}
)

set(STANDARD 20)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_STANDARD ${STANDARD}
               CMAKE_ARCHIVE_OUTPUT_DIRECTORY_<CONFIG> ${CMAKE_SOURCE_DIR}/lib
               CMAKE_LIBRARY_OUTPUT_DIRECTORY_<CONFIG> ${CMAKE_SOURCE_DIR}/lib
               CMAKE_RUNTIME_OUTPUT_DIRECTORY_<CONFIG> ${CMAKE_SOURCE_DIR}/bin
               OUTPUT_NAME "argfmt"
               DEBUG_POSTFIX "_d"
)

message("-- ${PROJECT_NAME} Has Finished Being Built")
