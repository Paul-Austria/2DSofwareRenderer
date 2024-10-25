# Top-level CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(OpenGLWrapper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add subdirectories for external libraries
add_subdirectory(external/glfw)
add_definitions(-DGLEW_STATIC)
add_subdirectory(external/glew)

find_package(OpenGL REQUIRED)
if(OpenGL_FOUND)
    message(STATUS "Found OpenGL: ${OPENGL_gl_LIBRARY}")
else()
    message(FATAL_ERROR "OpenGL not found")
endif()

# Add subdirectory for your SoftRendererLib
add_subdirectory(SoftRendererLib)

# Main OpenGL wrapper executable
add_executable(OpenGLWrapper OpenGLWrapper/main.cpp)

# Include directories for GLFW and SoftRendererLib
target_include_directories(OpenGLWrapper PRIVATE
    external/glfw/include
    ${CMAKE_SOURCE_DIR}/SoftRendererLib
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Link libraries for OpenGL wrapper
target_link_libraries(OpenGLWrapper SoftRendererLib libglew_static glfw ${OPENGL_gl_LIBRARY} ${CMAKE_DL_LIBS})

# Copy data for the OpenGLWrapper target
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLWrapper/data
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})

# Second target for testing SoftRendererLib without OpenGL
add_executable(SoftRendererTest SoftRendererTest/main.cpp)

# Include directories for the SoftRendererTest target
target_include_directories(SoftRendererTest PRIVATE
    ${CMAKE_SOURCE_DIR}/SoftRendererLib
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Link libraries for SoftRendererTest (no OpenGL)
target_link_libraries(SoftRendererTest SoftRendererLib)

# Copy data for the SoftRendererTest target if needed
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLWrapper/data
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})

# Set build-specific options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring for Debug build")
    # Add Debug specific options
    target_compile_definitions(OpenGLWrapper PRIVATE DEBUG)
    target_compile_definitions(SoftRendererTest PRIVATE DEBUG)
else()
    message(STATUS "Configuring for Release build")
    # Add Release specific options
    target_compile_definitions(OpenGLWrapper PRIVATE NDEBUG)
    target_compile_definitions(SoftRendererTest PRIVATE NDEBUG)
endif()

# Optionally, set default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
