cmake_minimum_required(VERSION 3.13)

# ---------------------------------------------------------------
# User Configurable Section
# ---------------------------------------------------------------

# Check if PICO_FRAMEWORK_PATH is set via environment
if(DEFINED ENV{PICO_FRAMEWORK_PATH})
    set(PICO_FRAMEWORK_PATH $ENV{PICO_FRAMEWORK_PATH})
    message(STATUS "Using PICO_FRAMEWORK_PATH from environment: ${PICO_FRAMEWORK_PATH}")
else()
    get_filename_component(PICO_FRAMEWORK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../" ABSOLUTE)
    message(STATUS "Using default PICO_FRAMEWORK_PATH: ${PICO_FRAMEWORK_PATH}")
endif()

# Board selection (default to pico2_w)
if(DEFINED ENV{PICO_BOARD})
    set(PICO_BOARD $ENV{PICO_BOARD})
else()
    set(PICO_BOARD pico2_w)
endif()

# Chip-specific memory map and flash offset for LittleFS

if(${PICO_BOARD} STREQUAL "pico2_w")
    set(LFS_FLASH_OFFSET 0x101C0000)
    set(MEMMAP_FRAGMENT ${CMAKE_SOURCE_DIR}/memmap_lfs_fragment_rp2350.ld)
else()
    set(LFS_FLASH_OFFSET 0x1C0000)
    set(MEMMAP_FRAGMENT ${CMAKE_SOURCE_DIR}/memmap_lfs_fragment_rp2040.ld)
endif()
message(STATUS "Using LittleFS offset: ${LFS_FLASH_OFFSET}")

# Flashing method: 'picoprobe' (default) or 'usb'
set(FLASH_METHOD "picoprobe" CACHE STRING "Select flashing method: picoprobe or usb")
message(STATUS "Using flash method: ${FLASH_METHOD}")

if(${FLASH_METHOD} STREQUAL "usb")
    message(WARNING "USB flashing requires BOOTSEL mode. Hold BOOTSEL while plugging in.")
endif()

# Wi-Fi credentials (optional from environment)
if(DEFINED ENV{WIFI_SSID})
    set(WIFI_SSID $ENV{WIFI_SSID})
endif()
if(DEFINED ENV{WIFI_PASSWORD})
    set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
endif()

# JWT secret (optional from environment)
if(DEFINED ENV{JWT_SECRET})
    set(JWT_SECRET_VALUE "$ENV{JWT_SECRET}")
else()
    set(JWT_SECRET_VALUE "your_secret")
    message(WARNING "JWT_SECRET environment variable not set. Using fallback value.")
endif()

# Pico Framework Feature Options
option(PICO_HTTP_ENABLE_LITTLEFS "Enable LittleFS-based StorageManager" OFF)
option(PICO_HTTP_ENABLE_HTTP_CLIENT "Enable HttpClient" ON)
option(PICO_TCP_ENABLE_TLS "Enable TLS support in TCP" ON)
option(PICO_HTTP_ENABLE_JWT "Enable JWT authentication support" OFF)
option(PICO_HTTP_TLS_VERIFY "Enable certificate verification for TLS connections" OFF)

# ---------------------------------------------------------------
# SDK and Framework Setup
# ---------------------------------------------------------------

if(NOT PICO_SDK_PATH AND NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "PICO_SDK_PATH is not set.")
endif()

if(NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message(FATAL_ERROR "FREERTOS_KERNEL_PATH is not set.")
endif()

include(../../cmake/pico_sdk_import.cmake)
include(../../cmake/FreeRTOS_Kernel_import.cmake)

pico_sdk_init()
set(FREERTOS_CONFIG_FILE_DIRECTORY ../../framework/include/port)
add_subdirectory(../../framework pico_framework)

project(pico_example_app C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ---------------------------------------------------------------
# LittleFS Image Setup
# ---------------------------------------------------------------

set(MKLITTLEFS_TOOL "${PICO_FRAMEWORK_PATH}/tools/mklittlefs/mklittlefs")
set(LFS_IMAGE "${CMAKE_BINARY_DIR}/littlefs.img")
set(LFS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/html")
set(LFS_TEMP_DIR "${CMAKE_BINARY_DIR}/html_out")
set(PICO_OPENOCD_EXECUTABLE "/Users/ianarchbell/.pico-sdk/openocd/0.12.0+dev/openocd")
set(PICO_OPENOCD_SCRIPTS "/Users/ianarchbell/.pico-sdk/openocd/0.12.0+dev/scripts")

file(GLOB_RECURSE HTML_FILES ${LFS_SOURCE_DIR}/*)

add_custom_command(
    OUTPUT ${LFS_IMAGE}
    COMMAND ${CMAKE_COMMAND} -E echo "Building LittleFS image..."
    COMMAND ${MKLITTLEFS_TOOL}
            -c ${LFS_SOURCE_DIR}
            -b 4096 -p 256 -s 262144
            ${LFS_IMAGE}
    DEPENDS ${HTML_FILES}
    COMMENT "Generating ${LFS_IMAGE} from ${LFS_SOURCE_DIR}"
)

# ---------------------------------------------------------------
# Application Target
# ---------------------------------------------------------------

add_executable(pico_example_app
    src/main.cpp
    src/App.cpp
)

if(PICO_HTTP_ENABLE_LITTLEFS)
    target_link_options(pico_example_app PRIVATE "-T${MEMMAP_FRAGMENT}")
else()
    target_sources(pico_example_app PRIVATE src/hw_config.c)
endif()

# Include paths
target_include_directories(pico_example_app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../framework/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../framework/include/port
)

# Board-specific FreeRTOS port
if(${PICO_BOARD} STREQUAL "pico2_w")
    target_include_directories(pico_example_app PRIVATE ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure/include)
else()
    target_include_directories(pico_example_app PRIVATE ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include)
endif()

# Linking and defines
target_link_libraries(pico_example_app
    pico_stdlib
    pico_framework
)

target_compile_definitions(pico_example_app PUBLIC
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    JWT_SECRET=\"${JWT_SECRET_VALUE}\"
    CYW43_HOST_NAME="Pico-Framework"
    ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_REQUIRED
    NO_SYS=0
)


target_compile_options(pico_example_app PRIVATE
    -Wno-unused-result
    -Wno-psabi
)

# USB/UART logging
pico_enable_stdio_usb(pico_example_app 0)
pico_enable_stdio_uart(pico_example_app 1)
pico_add_extra_outputs(pico_example_app)

# ---------------------------------------------------------------
# Flash Target (Unified)
# ---------------------------------------------------------------

add_custom_target(flash_all DEPENDS pico_example_app ${LFS_IMAGE})

if(${FLASH_METHOD} STREQUAL "usb")
    add_custom_command(TARGET flash_all POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Waiting for USB device in BOOTSEL mode..."
        COMMAND ${CMAKE_COMMAND} -E sleep 2
        COMMAND ${CMAKE_COMMAND} -E echo "Flashing firmware via USB..."
        COMMAND picotool load ${CMAKE_BINARY_DIR}/pico_example_app.uf2
        COMMAND ${CMAKE_COMMAND} -E echo "Flashing LittleFS image via USB..."
        COMMAND ${CMAKE_COMMAND} -E copy ${LFS_IMAGE} ${CMAKE_BINARY_DIR}/littlefs.bin
        COMMAND picotool load ${CMAKE_BINARY_DIR}/littlefs.bin --offset ${LFS_FLASH_OFFSET} 
    )
else()
    add_custom_command(TARGET flash_all POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Flashing firmware and filesystem via OpenOCD..."
        COMMAND ${PICO_OPENOCD_EXECUTABLE}
                -f ${PICO_OPENOCD_SCRIPTS}/interface/cmsis-dap.cfg
                -f ${PICO_OPENOCD_SCRIPTS}/target/rp2350.cfg
                -c "adapter speed 5000"
                -c "init"
                -c "reset init"
                -c "program ${CMAKE_BINARY_DIR}/pico_example_app.elf verify"
                -c "flash write_image erase ${LFS_IMAGE} ${LFS_FLASH_OFFSET} bin"
                -c "verify_image ${LFS_IMAGE} ${LFS_FLASH_OFFSET}"
                -c "resume"
                -c "exit"
    )
endif()

# ---------------------------------------------------------------
# Clean Targets
# ---------------------------------------------------------------

add_custom_target(clean_fs
    COMMAND ${CMAKE_COMMAND} -E remove -f ${LFS_IMAGE}
    COMMENT "Cleaning LittleFS image"
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaning all build outputs..."
    COMMAND ${CMAKE_COMMAND} -E remove -f
        ${CMAKE_BINARY_DIR}/pico_example_app.uf2
        ${CMAKE_BINARY_DIR}/pico_example_app.elf
        ${CMAKE_BINARY_DIR}/pico_example_app.bin
        ${CMAKE_BINARY_DIR}/pico_example_app.map
        ${LFS_IMAGE}
        ${CMAKE_BINARY_DIR}/littlefs.bin
    COMMENT "Removed app and filesystem images"
)
