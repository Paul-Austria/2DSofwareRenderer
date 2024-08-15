cmake_minimum_required(VERSION 3.10)
project(SoftRendererLib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# Include the sources file
include(CMakeSources.cmake)

# Define the library target
add_library(SoftRendererLib STATIC ${SOURCES})

# Define preprocessor macros for Debug and RelWithDebInfo configurations
target_compile_definitions(SoftRendererLib
    PUBLIC
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:DEBUG>
)

# Specify include directories for this library
target_include_directories(SoftRendererLib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

set_target_properties(SoftRendererLib PROPERTIES LINKER_LANGUAGE CXX)
