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

# ================================================================
# Rollercoasterometer: embed exact submodule versions (FreeRTOS + pico-sdk)
# ================================================================

find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  # Your own project (short commit + nice describe)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short=8 HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE OWN_COMMIT_SHORT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE OWN_DESCRIBE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )

  # FreeRTOS-Kernel submodule — change the path string if yours is not in root
  set(FREERTOS_SUBMODULE_PATH "FreeRTOS-Kernel")   # e.g. "lib/FreeRTOS-Kernel" if different
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD:${FREERTOS_SUBMODULE_PATH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE FREERTOS_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )

  # pico-sdk submodule — change if needed
  set(PICO_SDK_SUBMODULE_PATH "pico-sdk")
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse HEAD:${PICO_SDK_SUBMODULE_PATH}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE PICO_SDK_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )

  # Use Pico SDK's own version string if available (nice human-readable bonus)
  if(DEFINED PICO_SDK_VERSION_STRING)
    set(PICO_SDK_VERSION ${PICO_SDK_VERSION_STRING})
  else()
    set(PICO_SDK_VERSION "unknown")
  endif()
else()
  set(OWN_COMMIT_SHORT "unknown")
  set(OWN_DESCRIBE "unknown")
  set(FREERTOS_COMMIT "unknown")
  set(PICO_SDK_COMMIT "unknown")
  set(PICO_SDK_VERSION "unknown")
endif()

# Generate the manifest
set(MANIFEST_FILE "${CMAKE_CURRENT_BINARY_DIR}/build_manifest.txt")

file(WRITE ${MANIFEST_FILE}
"Git Manifest
================================
To get the exact submodule versions:

```
git clone git@github.com:kwan3217/rp2040_freertos_tutorial.git  # If not cloned yet
cd rp2040_freertos_tutorial
git stash # if there are local changes
git checkout <main-commit-from-manifest>
git stash pop # if stashed above
git submodule update --init --recursive
mkdir build; cd build
cmake ..
make
```

Since the hash of the version of the submodule(s) used is part of the repo, we will get the version
from the central repo for each submodule exactly matching

Main project:
  short commit : ${OWN_COMMIT_SHORT}
  describe     : ${OWN_DESCRIBE}

FreeRTOS-Kernel submodule:
  commit : ${FREERTOS_COMMIT}

pico-sdk submodule:
  commit         : ${PICO_SDK_COMMIT}
  version string : ${PICO_SDK_VERSION}

Built with:
  CMake ${CMAKE_VERSION}   ${CMAKE_BUILD_TYPE}
  Timestamp (configure) : ${CMAKE_CURRENT_BINARY_DIR}
"
)

# Make sure CMake reconfigures automatically if you update submodules
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/.gitmodules"
  "${CMAKE_CURRENT_SOURCE_DIR}/.git/index"
)

# Add the manifest to your existing tarball (no other changes needed!)
list(APPEND ATTACHED_FILES "${MANIFEST_FILE}")

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.S"      # assembly if you ever need it
)

# Remove anything that is not yours
list(FILTER PROJECT_SOURCES EXCLUDE REGEX  "/pico-sdk/|/FreeRTOS-Kernel/|/build/|CMakeFiles|\\.git|\\.cmake")

set(ATTACHED_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/memmap_default_sources.ld"
    "${CMAKE_CURRENT_SOURCE_DIR}/build/build_manifest.txt"
    # add more here – full paths or relative, doesn't matter
    # "${CMAKE_CURRENT_SOURCE_DIR}/calibration/..."
)

# Combine
set(ALL_PROJECT_FILES ${PROJECT_SOURCES} ${ATTACHED_FILES})

# Turn them into paths *relative* to the project root (so the tar has nice structure)
set(RELATIVE_FILES "")
foreach(file IN LISTS ALL_PROJECT_FILES)
    file(RELATIVE_PATH rel "${CMAKE_CURRENT_SOURCE_DIR}" "${file}")
    list(APPEND RELATIVE_FILES "${rel}")
endforeach()

set(TAR_FILE "${CMAKE_CURRENT_BINARY_DIR}/sources.tar.gz")

add_custom_command(
    OUTPUT "${TAR_FILE}"
    COMMAND tar -czf "${TAR_FILE}" -C "${CMAKE_CURRENT_SOURCE_DIR}" ${RELATIVE_FILES}
    DEPENDS ${ALL_PROJECT_FILES}
    COMMENT "Creating source tarball (${TAR_FILE})"
    VERBATIM
)

# 4. Turn the tar into an ELF object with a dedicated section
#    (RP2350 uses the same arm-none-eabi toolchain as RP2040)
set(TAR_OBJ "${CMAKE_CURRENT_BINARY_DIR}/sources.tar.o")

add_custom_command(
    OUTPUT "${TAR_OBJ}"
    COMMAND ${CMAKE_OBJCOPY}
        -I binary
        -O elf32-littlearm
        --binary-architecture arm
        --rename-section .data=.sources,alloc,load,readonly,data,contents
        "${TAR_FILE}"
        "${TAR_OBJ}"
    DEPENDS "${TAR_FILE}"
    COMMENT "Packing tarball in ELF object"
    VERBATIM
)

# 5. Make the object part of the final binary
add_custom_target(embed_sources ALL DEPENDS "${TAR_OBJ}")

# Tell CMake the object is generated and external
set_source_files_properties("${TAR_OBJ}" PROPERTIES
    GENERATED TRUE
    EXTERNAL_OBJECT TRUE
)

# Your main executable
add_executable(rp2040_freertos_tutorial
    main.c
)

# Add it to your executable (this makes the linker see it)
target_sources(rp2040_freertos_tutorial PRIVATE "${TAR_OBJ}")
add_dependencies(rp2040_freertos_tutorial embed_sources)


# Use our own copy of the linker script (now part of the source tarball)
pico_set_linker_script(rp2040_freertos_tutorial
    ${CMAKE_CURRENT_SOURCE_DIR}/memmap_default_sources.ld)


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
