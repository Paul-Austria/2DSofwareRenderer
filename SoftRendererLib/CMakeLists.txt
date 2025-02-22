cmake_minimum_required(VERSION 3.10)
project(SoftRendererLib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)


# custom options
option(SIMD_OPTIMIZED "SIMD OPTIMIZED" OFF)


# Include the sources from subdirectories
add_subdirectory(src)

# Define the library target
add_library(SoftRendererLib STATIC ${SOURCES})

# Define preprocessor macros for Debug and RelWithDebInfo configurations
target_compile_definitions(SoftRendererLib
    PUBLIC
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:DEBUG>
)

# Specify include directories for this library
target_include_directories(SoftRendererLib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
)


# Set optimization flags based on build type
if(CMAKE_BUILD_TYPE MATCHES Release)
    target_compile_options(SoftRendererLib PRIVATE 
        -O3                      # Enable full optimization
        -funroll-loops           # Unroll loops for better performance
    )
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    target_compile_options(SoftRendererLib PRIVATE 
        -O2                      # Enable standard optimization
        -g                       # Generate debug info
    )
endif()

set_target_properties(SoftRendererLib PROPERTIES LINKER_LANGUAGE CXX)

