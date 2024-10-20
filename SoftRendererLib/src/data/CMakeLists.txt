# Include subdirectory for PixelFormat
add_subdirectory(PixelFormat)

# Add source files from the data folder
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Texture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Color.cpp

)
set(SOURCES ${SOURCES} PARENT_SCOPE)
