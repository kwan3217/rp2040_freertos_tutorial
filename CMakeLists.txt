# CMakeLists.txt (top-level)
set(PICO_BOARD pico)

cmake_minimum_required(VERSION 3.13)

# Pull in Pico SDK (using the local submodule)
include(${CMAKE_CURRENT_LIST_DIR}/pico-sdk/pico_sdk_init.cmake)

# Project name
project(rp2040_freertos_tutorial C CXX ASM)

# Initialize the SDK
pico_sdk_init()

# Tell CMake where the source files are
add_executable(rp2040_freertos_tutorial
    main.c
)

# Link required Pico SDK libraries
target_link_libraries(rp2040_freertos_tutorial
    pico_stdlib              # Commonly used features (stdio, GPIO, etc.)
#    pico_cyw43_arch_none     # No Wi-Fi/Bluetooth needed for basic blinky
)

# Enable USB output + disable UART (we'll switch to USB CDC in the next step)
pico_enable_stdio_usb(rp2040_freertos_tutorial 1)
pico_enable_stdio_uart(rp2040_freertos_tutorial 0)

# Create map/bin/hex/uf2 output files
pico_add_extra_outputs(rp2040_freertos_tutorial)
