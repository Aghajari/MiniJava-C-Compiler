cmake_minimum_required(VERSION 3.23)
project(SimpleMiniJavaCompilerToC)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.h)
set(FILTERED_SOURCES)

foreach (SOURCE_FILE ${SOURCES})
    get_filename_component(FILENAME ${SOURCE_FILE} NAME)
    if (NOT SOURCE_FILE MATCHES "CMakeFiles/")
        if (NOT FILENAME MATCHES "^test.*")
            list(APPEND FILTERED_SOURCES ${SOURCE_FILE})
        endif ()
    endif ()
endforeach ()

message("Filtered sources: ${FILTERED_SOURCES}")

add_executable(
        ${PROJECT_NAME}
        ${FILTERED_SOURCES}
        generator/test/test_gen.cpp
)