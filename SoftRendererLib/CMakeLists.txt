cmake_minimum_required(VERSION 3.10)
project(SoftRendererLib VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard early
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

message(STATUS "Using Tergos2D Rendering engine")

# Custom options with better documentation
option(USE_NEON "Enable ARM NEON SIMD instructions" OFF)
option(ENABLE_ESP_SUPPORT "Enable ESP-IDF support" ON)

# ESP-IDF Integration
if(ENABLE_ESP_SUPPORT)
    set(ESP_IDF_PATH "/home/paul/esp/esp-idf/" CACHE PATH "Path to ESP-IDF installation")


endif()

# Include sources
add_subdirectory(src)

# Define library target
add_library(SoftRendererLib STATIC ${SOURCES})

# Debug definitions
target_compile_definitions(SoftRendererLib
    PUBLIC
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:DEBUG>
    $<$<BOOL:${USE_NEON}>:USE_NEON>
)

# Include directories
target_include_directories(SoftRendererLib
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/include"
)
if(ENABLE_ESP_SUPPORT)
    add_definitions(-DENABLE_ESP_SUPPORT=1)
    set(ESP_PLATFORM 1)
    if(EXISTS "${ESP_IDF_PATH}/components/esp_common/include")
        target_include_directories(SoftRendererLib PRIVATE
            "${ESP_IDF_PATH}/components/esp_common/include/"
            "${ESP_IDF_PATH}/components/esp_hw_support/include/"
            "${ESP_IDF_PATH}/components/soc/esp32s3/include/"
            "${ESP_IDF_PATH}/components/xtensa/include/"
            "${ESP_IDF_PATH}/components/xtensa/esp32s3/include/"
            "${ESP_IDF_PATH}/components/heap/include/"
            "${ESP_IDF_PATH}/components/log/include"
            "${ESP_IDF_PATH}/components/esp_rom/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/common/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/math/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/math/add/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/math/addc/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/math/mul/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/math/mulc/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/iir/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/math/sqrt/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/math/sub/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/fir/include/"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/dotprod/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/windows/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/windows/hann/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/windows/blackman/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/windows/blackman_harris/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/windows/blackman_nuttall/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/windows/flat_top/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/windows/nuttall/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/conv/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../../managed_components/espressif__esp-dsp/modules/support/include"


            "${CMAKE_BINARY_DIR}/config"
        )
        add_definitions(-DESP_PLATFORM)
        message(STATUS "ESP-IDF support enabled at ${ESP_IDF_PATH}")
    else()
        message(WARNING "ESP-IDF path not found at ${ESP_IDF_PATH}")
    endif()
endif()



# Platform-specific optimizations
include(CheckCXXCompilerFlag)

# Optimization flags based on build type and architecture
if(CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "Building for release")

    set(OPTIMIZATION_FLAGS
        -O3
        -funroll-loops
        -ffast-math
        -ftree-vectorize
    )

    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|aarch64)")
        if(USE_NEON)
            check_cxx_compiler_flag(-mfpu=neon COMPILER_SUPPORTS_NEON)
            if(COMPILER_SUPPORTS_NEON)
                list(APPEND OPTIMIZATION_FLAGS
                    -mfpu=neon
                    -mneon-for-64bits
                )
                message(STATUS "ARM NEON optimizations enabled")
            endif()
        endif()
    endif()

    # Apply optimization flags
    target_compile_options(SoftRendererLib PRIVATE ${OPTIMIZATION_FLAGS})

elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    target_compile_options(SoftRendererLib PRIVATE
        -O2
        -g
        -ftree-vectorize
    )
endif()

# Additional properties
set_target_properties(SoftRendererLib PROPERTIES
    LINKER_LANGUAGE CXX
    POSITION_INDEPENDENT_CODE ON  # Add -fPIC
)

# Export compile commands for tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Version information
target_compile_definitions(SoftRendererLib PRIVATE
    SOFTRENDERER_VERSION="${PROJECT_VERSION}"
    SOFTRENDERER_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    SOFTRENDERER_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    SOFTRENDERER_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)