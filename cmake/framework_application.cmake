# framework_application.cmake
#
# author: Ian Archbell
# date: 2025-05-25
# description: CMake script for building applications using the Pico Framework
# license: MIT License
# 
# Shared CMake logic for all framework-based applications
#
# It handles:
#
# Required preconditions -Fails early if APP_SOURCES missing
# Standard versions	- C11 / C++17 set globally here
# FreeRTOS handling	- Checks both variable and environment fallback
# Framework inclusion - Includes it before target is created
# Target creation - Centralized add_executable(${APP_NAME} ${APP_SOURCES})
# LittleFS logic - Generates .ld linker script file if enabled with proper offsets and size specified by user
# Includes - Adds framework and FreeRTOS includes to target
# Target property assignment - Proper sequencing of target_link_libraries, target_compile_definitions
# Flashing logic - Fully handled USB/Picotool + Picoprobe/OpenOCD paths, offset safe, image built
# Clean targets	- Enables either the filesystem or all to be deleted, so clean_fs, clean_all are provided
# Extensibility	- External apps can set values before include and customize afterward

# ----------------------------------------------------------------------------------
# Preconditions
# ----------------------------------------------------------------------------------

if(NOT DEFINED APP_SOURCES)
    message(FATAL_ERROR "APP_SOURCES must be defined before including framework_application.cmake")
endif()

if(NOT DEFINED APP_NAME)
    message(FATAL_ERROR "APP_NAME must be defined before including framework_application.cmake")
endif()

if(NOT DEFINED PICO_BOARD)
    message(FATAL_ERROR "PICO_BOARD must be defined before including framework_application.cmake")
endif()

if(NOT DEFINED PICO_HTTP_ENABLE_LITTLEFS)
    set(PICO_HTTP_ENABLE_LITTLEFS FALSE)
endif()

if(PICO_HTTP_ENABLE_LITTLEFS)
    if(NOT DEFINED LFS_PARTITION_SIZE)
        set(LFS_PARTITION_SIZE 0x40000)  # Default to 256KB
    endif()

    elseif(${LFS_PARTITION_SIZE} EQUAL 0)
        message(FATAL_ERROR "LFS_PARTITION_SIZE is 0 — this is invalid. Please set a non-zero size for the LittleFS partition.")
    endif()
#endif

if(NOT DEFINED FLASH_METHOD)
    set(FLASH_METHOD usb) 
else()
    string(TOLOWER "${FLASH_METHOD}" FLASH_METHOD)
    if(NOT FLASH_METHOD STREQUAL "usb" AND NOT FLASH_METHOD STREQUAL "openocd")
        message(FATAL_ERROR "FLASH_METHOD must be either 'usb' or 'openocd'")
    endif()
endif()

# ----------------------------------------------------------------------------------
# Flash Method Detection and Normalization
# ----------------------------------------------------------------------------------

if(NOT DEFINED FLASH_METHOD)
    set(FLASH_METHOD usb)  # Default to USB
else()
    string(TOLOWER "${FLASH_METHOD}" FLASH_METHOD)
    if(NOT FLASH_METHOD STREQUAL "usb" AND NOT FLASH_METHOD STREQUAL "openocd")
        message(FATAL_ERROR "FLASH_METHOD must be either 'usb' or 'openocd'")
    endif()
endif()

# ----------------------------------------------------------------------------------
# Tool Lookup for Flashing and Filesystem Tools
# ----------------------------------------------------------------------------------

if(FLASH_METHOD STREQUAL "usb")
    if(NOT DEFINED PICOTOOL_EXECUTABLE)
        find_program(PICOTOOL_EXECUTABLE NAMES picotool)
    endif()
    if(NOT PICOTOOL_EXECUTABLE)
        message(FATAL_ERROR "FLASH_METHOD is 'usb', but 'picotool' was not found in PATH. Please install or provide -DPICOTOOL_EXECUTABLE")
    endif()
    message(STATUS "Using picotool: ${PICOTOOL_EXECUTABLE}")
endif()

if(FLASH_METHOD STREQUAL "openocd")
    if(NOT OPENOCD_EXECUTABLE)
        message(STATUS "System PATH seen by CMake: $ENV{PATH}")
        find_program(OPENOCD_EXECUTABLE NAMES openocd)
    endif()
    if(NOT OPENOCD_EXECUTABLE)
        message(FATAL_ERROR "FLASH_METHOD is 'openocd', but 'openocd' was not found in PATH. Please install or provide -DOPENOCD_EXECUTABLE")
    endif()
    message(STATUS "Using openocd: ${OPENOCD_EXECUTABLE}")
endif()

# ----------------------------------------------------------------------------------
# Basic setup
# ----------------------------------------------------------------------------------

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ----------------------------------------------------------------------------------
# Required FreeRTOS
# ----------------------------------------------------------------------------------

if(NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message(FATAL_ERROR "FREERTOS_KERNEL_PATH is not set.")
endif()

include(${CMAKE_SOURCE_DIR}/../../cmake/FreeRTOS_Kernel_import.cmake)

# ----------------------------------------------------------------------------------
# Add framework first so its dependencies are resolved before app is built
# ----------------------------------------------------------------------------------

set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_SOURCE_DIR}/../../framework/include/port)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../framework pico_framework)

# ----------------------------------------------------------------------------------
# Declare the executable from APP_SOURCES (after framework is fully defined)
# ----------------------------------------------------------------------------------

add_executable(${APP_NAME} ${APP_SOURCES})

# ----------------------------------------------------------------------------------
# LittleFS offset and linker fragment (generated if feature is enabled)
# ----------------------------------------------------------------------------------

set(HAS_VALID_LFS_CONFIG FALSE)

if(${PICO_BOARD} STREQUAL "pico2_w")
    set(PICO_TARGET_CFG_FILE "rp2350.cfg")
elseif(${PICO_BOARD} STREQUAL "pico_w")
    set(PICO_TARGET_CFG_FILE "rp2040.cfg")
else()
    message(FATAL_ERROR "Unsupported board: ${PICO_BOARD}")
endif()

if(PICO_HTTP_ENABLE_LITTLEFS)

    # Define flash size per board
    set(FLASH_TOTAL_SIZE 0x200000)  # 2MB default for pico_w
    if(${PICO_BOARD} STREQUAL "pico2_w")
        set(FLASH_TOTAL_SIZE 0x400000)  # 4MB
    endif()

    # Compute absolute flash region
    math(EXPR LFS_FLASH_END "0x10000000 + ${FLASH_TOTAL_SIZE}")
    math(EXPR LFS_FLASH_START "${LFS_FLASH_END} - ${LFS_PARTITION_SIZE}")

    # Convert to proper hex strings
    execute_process(
        COMMAND printf "0x%08X" ${LFS_FLASH_START}
        OUTPUT_VARIABLE LFS_FLASH_START_HEX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND printf "0x%X" ${LFS_PARTITION_SIZE}
        OUTPUT_VARIABLE LFS_PARTITION_SIZE_HEX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND printf "0x%08X" ${LFS_FLASH_END}
        OUTPUT_VARIABLE LFS_FLASH_END_HEX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Generate the linker fragment
    set(GENERATED_LD "${CMAKE_BINARY_DIR}/generated_lfs_fragment.ld")
    file(WRITE ${GENERATED_LD}
        "MEMORY {
"
        "  LFS (r) : ORIGIN = ${LFS_FLASH_START_HEX}, LENGTH = ${LFS_PARTITION_SIZE_HEX}
"
        "}
"
        "__flash_lfs_start = ORIGIN(LFS);
"
        "__flash_lfs_end = ORIGIN(LFS) + LENGTH(LFS);
"
    )

    message(STATUS "Using generated linker fragment: ${GENERATED_LD}")
    message(STATUS "Using LittleFS start: ${LFS_FLASH_START_HEX}")
    message(STATUS "Using LittleFS end:   ${LFS_FLASH_END_HEX}")

    set(MEMMAP_FRAGMENT ${GENERATED_LD})
    set(LFS_FLASH_OFFSET ${LFS_FLASH_START_HEX})
    set(HAS_VALID_LFS_CONFIG TRUE)

    # Optionally build the image if html/ exists
    set(LFS_IMAGE "${CMAKE_BINARY_DIR}/littlefs.img")
    set(LFS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/html")

    if(EXISTS ${LFS_SOURCE_DIR})
        if (DEFINED ENV{MKLITTLEFS_EXECUTABLE})
            message(STATUS "Using MKLITTLEFS_EXECUTABLE from environment: $ENV{MKLITTLEFS_EXECUTABLE}")
            set(MKLITTLEFS_EXECUTABLE $ENV{MKLITTLEFS_EXECUTABLE})
        endif()

        if(NOT DEFINED MKLITTLEFS_EXECUTABLE)
            find_program(MKLITTLEFS_EXECUTABLE NAMES mklittlefs)
        endif()

        if(NOT MKLITTLEFS_EXECUTABLE)
            message(FATAL_ERROR "mklittlefs is required to build the LittleFS image, but was not found. Please install or set -DMKLITTLEFS_EXECUTABLE.")
        endif()

        message(STATUS "Using mklittlefs: ${MKLITTLEFS_EXECUTABLE}")

        file(GLOB_RECURSE HTML_FILES ${LFS_SOURCE_DIR}/*)

        add_custom_command(
            OUTPUT ${LFS_IMAGE}
            COMMAND ${CMAKE_COMMAND} -E echo "Building LittleFS image..."
            COMMAND ${MKLITTLEFS_EXECUTABLE}
                    -c ${LFS_SOURCE_DIR}
                    -b 4096 -p 256 -s ${LFS_PARTITION_SIZE}
                    ${LFS_IMAGE}
            DEPENDS ${HTML_FILES}
            COMMENT "Generating ${LFS_IMAGE} from ${LFS_SOURCE_DIR}"
        )

        add_custom_target(build_lfs_image ALL DEPENDS ${LFS_IMAGE})
    else()
        message(STATUS "[framework] html/ not found, skipping LFS image build but keeping linker symbols.")
    endif()
endif()


# ----------------------------------------------------------------------------------
# Includes
# ----------------------------------------------------------------------------------

target_include_directories(${APP_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/../../framework/include
    ${CMAKE_SOURCE_DIR}/../../framework/include/port
)

if(${PICO_BOARD} STREQUAL "pico2_w")
    target_include_directories(${APP_NAME} PRIVATE ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure/include)
else()
    target_include_directories(${APP_NAME} PRIVATE ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include)
endif()

# ----------------------------------------------------------------------------------
# Linking and linker fragment
# ----------------------------------------------------------------------------------

target_link_libraries(${APP_NAME}
    pico_framework
)

if(PICO_HTTP_ENABLE_LITTLEFS)
    message(STATUS "Linking with MEMMAP_FRAGMENT: ${MEMMAP_FRAGMENT}")
    if(DEFINED MEMMAP_FRAGMENT)
        target_link_options(${APP_NAME} PRIVATE "-T${MEMMAP_FRAGMENT}")
    endif()
endif()

# ----------------------------------------------------------------------------------
# Compiler definitions
# ----------------------------------------------------------------------------------

target_compile_definitions(${APP_NAME} PUBLIC
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    CYW43_HOST_NAME="Pico-Framework"
    ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_REQUIRED
    NO_SYS=0
)

target_compile_options(${APP_NAME} PRIVATE
    -Wno-unused-result
    -Wno-psabi
)


# ------------------------------------------------------------------------------
# OpenOCD Script Directory Resolution
# ------------------------------------------------------------------------------

# Allow user override
if(NOT DEFINED PICO_OPENOCD_SCRIPTS)

    # Try system-installed openocd (Homebrew, Linux package, etc.)
    find_program(OPENOCD_EXECUTABLE NAMES openocd)
    if(NOT OPENOCD_EXECUTABLE)
        message(FATAL_ERROR "OpenOCD executable not found. Please install or add to PATH.")
    endif()

    # Try calling openocd with no -s, just to test if it can find scripts
    execute_process(
        COMMAND ${OPENOCD_EXECUTABLE}
                -f interface/cmsis-dap.cfg
                -c "exit"
        RESULT_VARIABLE OPENOCD_PROBE_RESULT
        OUTPUT_QUIET ERROR_QUIET
    )

    if(OPENOCD_PROBE_RESULT EQUAL 0)
        message(STATUS "OpenOCD appears to find its scripts automatically. No script path override will be set.")
        # Do NOT set PICO_OPENOCD_SCRIPTS
    else()
        # Try relative to OpenOCD binary
        get_filename_component(OPENOCD_DIR ${OPENOCD_EXECUTABLE} DIRECTORY)
        set(POSSIBLE_SCRIPT_DIR "${OPENOCD_DIR}/../share/openocd/scripts")

        if(EXISTS "${POSSIBLE_SCRIPT_DIR}/interface/cmsis-dap.cfg")
            set(PICO_OPENOCD_SCRIPTS "${POSSIBLE_SCRIPT_DIR}")
            message(STATUS "Using OpenOCD scripts from: ${PICO_OPENOCD_SCRIPTS}")
        endif()

        # Try vendored fallback if needed
        if(NOT DEFINED PICO_OPENOCD_SCRIPTS)
            set(VENDORED_OCD_DIR "${CMAKE_SOURCE_DIR}/../../framework/tools/openocd-scripts")
            if(EXISTS "${VENDORED_OCD_DIR}/interface/cmsis-dap.cfg")
                set(PICO_OPENOCD_SCRIPTS "${VENDORED_OCD_DIR}")
                message(STATUS "Using vendored OpenOCD scripts from: ${PICO_OPENOCD_SCRIPTS}")
            endif()
        endif()

        if(NOT DEFINED PICO_OPENOCD_SCRIPTS)
            message(WARNING "OpenOCD could not locate its scripts. Set PICO_OPENOCD_SCRIPTS manually if flashing fails.")
        endif()
    endif()

endif()

# ----------------------------------------------------------------------------------
# Flash Target
# ----------------------------------------------------------------------------------

if(NOT TARGET flash_all)

    set(LFS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/html")
    set(INCLUDE_FS_FLASH FALSE)

    if(PICO_HTTP_ENABLE_LITTLEFS AND EXISTS ${LFS_SOURCE_DIR})
        set(INCLUDE_FS_FLASH TRUE)
    endif()

    if(INCLUDE_FS_FLASH)
        add_custom_target(flash_all DEPENDS ${APP_NAME} build_lfs_image)
    else()
        add_custom_target(flash_all DEPENDS ${APP_NAME})
    endif()

    if(${FLASH_METHOD} STREQUAL "usb")
        if(INCLUDE_FS_FLASH)
            add_custom_command(TARGET flash_all POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Flashing application via USB..."
                COMMAND ${PICOTOOL_EXECUTABLE} load -f ${CMAKE_BINARY_DIR}/${APP_NAME}.uf2
                COMMAND ${CMAKE_COMMAND} -E echo "Flashing LittleFS image..."
                COMMAND ${CMAKE_COMMAND} -E copy ${LFS_IMAGE} ${CMAKE_BINARY_DIR}/littlefs.bin
                COMMAND ${PICOTOOL_EXECUTABLE} load ${CMAKE_BINARY_DIR}/littlefs.bin --offset ${LFS_FLASH_OFFSET}
            )
        else()
            add_custom_command(TARGET flash_all POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Flashing application via USB..."
                COMMAND ${PICOTOOL_EXECUTABLE} load -f ${CMAKE_BINARY_DIR}/${APP_NAME}.uf2
            )
        endif()
    else()

        message(STATUS "Using OpenOCD scripts from: ${PICO_OPENOCD_SCRIPTS}")

        if(DEFINED PICO_OPENOCD_SCRIPTS)
            set(OCD_INTERFACE_FILE "${PICO_OPENOCD_SCRIPTS}/interface/cmsis-dap.cfg")
            set(OCD_TARGET_FILE "${PICO_OPENOCD_SCRIPTS}/target/${PICO_TARGET_CFG_FILE}")
        else()
            set(OCD_INTERFACE_FILE "interface/cmsis-dap.cfg")
            set(OCD_TARGET_FILE "target/${PICO_TARGET_CFG_FILE}")
        endif()
        
        if(INCLUDE_FS_FLASH)
        
            add_custom_command(TARGET flash_all POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Flashing application via OpenOCD..."
                COMMAND ${OPENOCD_EXECUTABLE}
                        -f ${OCD_INTERFACE_FILE}
                        -f ${OCD_TARGET_FILE}
                        -c "adapter speed 5000"
                        -c "init"
                        -c "reset init"
                        -c "program ${CMAKE_BINARY_DIR}/${APP_NAME}.elf verify"
                        -c "flash write_image erase ${LFS_IMAGE} ${LFS_FLASH_OFFSET} bin"
                        -c "verify_image ${LFS_IMAGE} ${LFS_FLASH_OFFSET}"
                        -c "resume"
                        -c "exit"
            )
        else()
            add_custom_command(TARGET flash_all POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Flashing application via OpenOCD..."
                COMMAND ${OPENOCD_EXECUTABLE}
                        -f ${OCD_INTERFACE_FILE}
                        -f ${OCD_TARGET_FILE}
                        -c "adapter speed 5000"
                        -c "init"
                        -c "reset init"
                        -c "program ${CMAKE_BINARY_DIR}/${APP_NAME}.elf verify"
                        -c "resume"
                        -c "exit"
            )
        endif()
    endif()

endif()

# ----------------------------------------------------------------------------------
# Clean Targets
# ----------------------------------------------------------------------------------

add_custom_target(clean_fs
    COMMAND ${CMAKE_COMMAND} -E remove -f ${LFS_IMAGE}
    COMMENT "Cleaning LittleFS image"
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaning all build outputs..."
    COMMAND ${CMAKE_COMMAND} -E remove -f
        ${CMAKE_BINARY_DIR}/${APP_NAME}.uf2
        ${CMAKE_BINARY_DIR}/${APP_NAME}.elf
        ${CMAKE_BINARY_DIR}/${APP_NAME}.bin
        ${CMAKE_BINARY_DIR}/${APP_NAME}.map
        ${LFS_IMAGE}
        ${CMAKE_BINARY_DIR}/littlefs.bin
    COMMENT "Removed app and filesystem images"
)
