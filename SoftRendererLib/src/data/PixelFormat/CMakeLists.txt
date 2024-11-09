# Add source files from PixelFormat folder
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/PixelConverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PlatformInd/PixelConverter.cpp
)


if(SIMD_OPTIMIZED)
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Optimized/PixelConverter.cpp
)
else()

set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/PlatformInd/PixelConverter.cpp
)

endif()


set(SOURCES ${SOURCES} PARENT_SCOPE)
