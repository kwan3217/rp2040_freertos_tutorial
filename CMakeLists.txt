# CMakeLists.txt (top-level)
cmake_minimum_required(VERSION 3.13)

# Generate .d dependency files for easier header chasing
set(CMAKE_DEPENDENCY_FILE_SUFFIX ".d")

set(PICO_BOARD pico)

# Pull in Pico SDK (using the local submodule)
set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/pico-sdk)
include(${CMAKE_CURRENT_LIST_DIR}/pico-sdk/external/pico_sdk_import.cmake)

project(rp2040_freertos_tutorial C CXX ASM)

# Initialize the SDK (this sets PICO_SDK_PATH etc.)
pico_sdk_init()

# Point to your FreeRTOS-Kernel submodule
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS-Kernel CACHE PATH "")

# Optional: explicitly choose heap implementation (4 is common/recommended)
set(FREERTOS_HEAP 4 CACHE STRING "")

# Include the official RP2040 port import script (after pico_sdk_init!)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# Create a small interface lib to expose FreeRTOSConfig.h to anything that needs it
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}   # project root → FreeRTOSConfig.h
)

set(ATTACHED_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
    # add more here – full paths or relative, doesn't matter
    # "${CMAKE_CURRENT_SOURCE_DIR}/calibration/..."
)

# Your main executable
add_executable(rp2040_freertos_tutorial
    main.c
)


# After add_executable(rollercoasterometer ...)

include(cmake/create_embedded_tarball.cmake)


create_embedded_tarball(
    TARGET       rp2040_freertos_tutorial
    EXTRA_FILES  ${ATTACHED_FILES}
    OUTPUT_OBJ   TAR_OBJ
)

target_sources(rp2040_freertos_tutorial PRIVATE sources.S)

set_source_files_properties(sources.S PROPERTIES
    COMPILE_FLAGS "-I${CMAKE_CURRENT_BINARY_DIR}"
    OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/sources.tar.gz"
)

# Link everything
target_link_libraries(rp2040_freertos_tutorial PRIVATE
    pico_stdlib
#    pico_cyw43_arch_none          # no Wi-Fi/BT needed for basic tests
    freertos_config               # makes FreeRTOSConfig.h visible
    FreeRTOS-Kernel               # core kernel (created by import script)
    FreeRTOS-Kernel-Heap4         # matches FREERTOS_HEAP=4
)

# USB CDC for printf, no UART
pico_enable_stdio_usb(rp2040_freertos_tutorial 1)
pico_enable_stdio_uart(rp2040_freertos_tutorial 0)

# Generate extra outputs (.uf2, .map, etc.)
pico_add_extra_outputs(rp2040_freertos_tutorial)
