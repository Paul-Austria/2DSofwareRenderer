# Top-level CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(OpenGLWrapper)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(external/glfw)
add_definitions(-DGLEW_STATIC)
add_subdirectory(external/glew)

find_package(OpenGL REQUIRED)
if(OpenGL_FOUND)
    message(STATUS "Found OpenGL: ${OPENGL_gl_LIBRARY}")
else()
    message(FATAL_ERROR "OpenGL not found")
endif()

add_subdirectory(SoftRendererLib)

add_executable(OpenGLWrapper OpenGLWrapper/main.cpp)

target_link_libraries(OpenGLWrapper SoftRendererLib libglew_static glfw ${OPENGL_gl_LIBRARY} ${CMAKE_DL_LIBS})

# Include directories for GLFW and SoftRendererLib
target_include_directories(OpenGLWrapper PRIVATE
    external/glfw/include
    ${CMAKE_SOURCE_DIR}/SoftRendererLib
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLWrapper/data
     DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug)
