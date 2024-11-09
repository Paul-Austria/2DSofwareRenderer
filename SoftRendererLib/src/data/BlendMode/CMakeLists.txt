# Add source files from PixelFormat folder
set(SOURCES
    ${SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/BlendMode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BlendFunctions.cpp

)


if(SIMD_OPTIMZED)
set(SOURCES
    ${SOURCES}
)
else()

set(SOURCES
    ${SOURCES}
)

endif()


set(SOURCES ${SOURCES} PARENT_SCOPE)
