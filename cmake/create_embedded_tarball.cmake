# cmake/CreateEmbeddedTarball.cmake
# Low-level archive builder: files + manifest + tar.gz + ELF object
# Reusable for any rollercoasterometer firmware or derivative project

include_guard()

function(create_embedded_tarball)
    ### arguments:
    ###   :param TARGET: Name of target, used as prefix for intermediate files. required.
    ###   :param SECTION_NAME: name of section in elf object, ".sorces" by default if not passed
    ###   :param OUTPUT_OBJ: name of elf object file to create. This elf object will have one section with the contents
    ###                      of the archive as read only data
    cmake_parse_arguments(
        PARSE_ARGV 0
        ARG
        ""
        "TARGET;SECTION_NAME;OUTPUT_OBJ"
        "EXTRA_FILES"
    )

    if(NOT ARG_TARGET)
        message(FATAL_ERROR "create_embedded_tarball: TARGET is required")
    endif()
    if(NOT ARG_SECTION_NAME)
        set(ARG_SECTION_NAME ".sources")
    endif()
    if(NOT ARG_OUTPUT_OBJ)
        set(ARG_OUTPUT_OBJ "EMBEDDED_TAR_OBJ")
    endif()

    set(TARGET ${ARG_TARGET})

    # Collect every file you own (sources, headers, linker script, this module itself…)
    file(GLOB_RECURSE PROJECT_FILES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.S"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.ld"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cmake"
    )

    # Exclude SDKs, build artifacts, git
    list(FILTER PROJECT_FILES EXCLUDE REGEX
        "/(pico-sdk|FreeRTOS-Kernel|build|CMakeFiles|cmake-build-debug|\\.git)/")

    # combine
    set(ALL_FILES ${PROJECT_FILES} ${ARG_EXTRA_FILES})

    # Relative paths for clean tar layout
    set(RELATIVE_FILES "")
    foreach(f IN LISTS ALL_FILES)
        file(RELATIVE_PATH rel "${CMAKE_CURRENT_SOURCE_DIR}" "${f}")
        list(APPEND RELATIVE_FILES "${rel}")
    endforeach()

    # Reproducible manifest (exact submodule SHAs)
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short=8 HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE MAIN_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE MAIN_DESCRIBE OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD:pico-sdk
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE PICO_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD:FreeRTOS-Kernel
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE FREERTOS_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    else()
        set(MAIN_COMMIT "unknown")
        set(MAIN_DESCRIBE "unknown")
        set(PICO_COMMIT "unknown")
        set(FREERTOS_COMMIT "unknown")
    endif()

    set(MANIFEST "${CMAKE_CURRENT_BINARY_DIR}/build_manifest.txt")
    file(WRITE "${MANIFEST}"
"Build Manifest - ${TARGET}
=====================================

Main project:    ${MAIN_COMMIT}  (${MAIN_DESCRIBE})
pico-sdk:        ${PICO_COMMIT}
FreeRTOS-Kernel: ${FREERTOS_COMMIT}
CMake:           ${CMAKE_VERSION}  ${CMAKE_BUILD_TYPE}
")

    # Create tarball (your sources + manifest)
    set(TAR_BASE "${CMAKE_CURRENT_BINARY_DIR}/sources.tar")
    set(TAR_FILE "${TAR_BASE}.gz")

    add_custom_command(
        OUTPUT "${TAR_FILE}"
        COMMAND tar -cf "${TAR_BASE}" -C "${CMAKE_CURRENT_SOURCE_DIR}" ${RELATIVE_FILES}
        COMMAND tar -rf "${TAR_BASE}" --transform "s|.*|build_manifest.txt|" "${MANIFEST}"
        COMMAND gzip -9 -f "${TAR_BASE}"
        DEPENDS ${ALL_FILES} "${MANIFEST}"
        COMMENT "Creating source tarball (${TAR_FILE})"
        VERBATIM
    )

    set(${ARG_OUTPUT_OBJ} "${TAR_FILE}" PARENT_SCOPE)
endfunction()