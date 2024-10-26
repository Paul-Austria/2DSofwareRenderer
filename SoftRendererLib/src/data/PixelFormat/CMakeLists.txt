# Add source files from PixelFormat folder
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/PixelConverter.cpp

)


if(SIMD_OPTIMZED)
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/Optimized/PixelConverter.cpp
)
else()

set(SOURCES
    ${SOURCES}
)

endif()


set(SOURCES ${SOURCES} PARENT_SCOPE)
