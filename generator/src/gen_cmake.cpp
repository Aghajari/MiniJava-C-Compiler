#include "../internal/generator_internal.h"

/**
 * @brief Writes a default `CMakeLists.txt` configuration for building the generated project.
 *
 * This function generates a standard `CMakeLists.txt` file to configure and build the
 * generated project with CMake. The generated build system ensures:
 * - All `.c` and `.h` files in the project directory are included in the build process.
 * - Filters out temporary or irrelevant files (like those in `CMakeFiles/`) from the source list.
 * - Configures the project to be built with C99 standard compliance.
 *
 * @note This function writes the `CMakeLists.txt` file directly to disk.
 */
void write_cmake() {
    auto cmake = "cmake_minimum_required(VERSION 3.23)\n"
                 "\n"
                 "project(CompiledProject LANGUAGES C)\n"
                 "\n"
                 "set(CMAKE_C_STANDARD 99)\n"
                 "\n"
                 "file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/*.c ${CMAKE_SOURCE_DIR}/*.h)\n"
                 "set(FILTERED_SOURCES)\n"
                 "\n"
                 "foreach (SOURCE_FILE ${SOURCES})\n"
                 "    get_filename_component(FILENAME ${SOURCE_FILE} NAME)\n"
                 "    if (NOT SOURCE_FILE MATCHES \"CMakeFiles/\")\n"
                 "        list(APPEND FILTERED_SOURCES ${SOURCE_FILE})\n"
                 "    endif ()\n"
                 "endforeach ()\n"
                 "\n"
                 "add_executable(${PROJECT_NAME} ${FILTERED_SOURCES})";
    write_file("CMakeLists.txt", cmake);
}