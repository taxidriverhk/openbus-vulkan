# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("OpenBus")

set (CMAKE_CXX_STANDARD 17)

# Version number generation
set (VERSION_MAJOR 0)
set (VERSION_MINOR 1)
set (VERSION_PATCH 1)

execute_process(COMMAND
    git describe --always --abbrev=15
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_SHA1
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")

# Qt configuration
set (CMAKE_AUTOMOC ON)
set (CMAKE_AUTOUIC ON)
set (CMAKE_AUTORCC ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)

# SFML configuration
set (SFML_DIR "${LIBRARY_DIR}/sfml/lib/cmake/SFML")

# Shaderc configuration
set (ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set (SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set (SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)

set (SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
set (RESOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resource")
set (LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Library")

# Include sub-projects.
add_subdirectory ("${SOURCE_DIR}")
add_subdirectory ("${LIBRARY_DIR}/fmt")
add_subdirectory ("${LIBRARY_DIR}/glfw")
add_subdirectory ("${LIBRARY_DIR}/glm")
add_subdirectory ("${LIBRARY_DIR}/plog")
add_subdirectory ("${LIBRARY_DIR}/shaderc")
