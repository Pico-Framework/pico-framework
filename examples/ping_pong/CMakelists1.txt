# CMakeLists.txt 
# author: Ian Archbell
# date: 2025-05-25
# description: CMakeLists.txt for a minimal Pico application using the Pico SDK
# This file is part of the Pico HTTP framework. 
# It uses framework_application.cmake to simplify the build process.
# This file is designed to be used as a template for your own applications.
# license: MIT License

# ------------------------------------------------------------------------------
# Toolchain and Dependencies (Must Be Installed by the User)
# ------------------------------------------------------------------------------

# - CMake (version 3.13 or later)
#   Required for configuring and generating project build files.
#   Not installed by default on most platforms:
#     - macOS: install via Homebrew (`brew install cmake`)
#     - Linux: install via package manager (e.g. `sudo apt install cmake`)
#              May be an older version — consider building manually if needed
#     - Windows: install from https://cmake.org/download or via Chocolatey (`choco install cmake`)
#   Ensure CMake is available in your system PATH.

# - GNU Arm Embedded Toolchain (version 10.3 or later)
#   Required to compile firmware for RP2040-based boards.
#   Download from: https://developer.arm.com/downloads/-/gnu-rm
#   Install and ensure `arm-none-eabi-gcc` and related tools are in your PATH.

# - Pico SDK (version 1.5.0 or later)
#   Core SDK for Raspberry Pi Pico development.
#   Must be cloned and available for CMake to reference.
#   Repository: https://github.com/raspberrypi/pico-sdk

# - FreeRTOS Kernel (official version, latest stable)
#   Required for all framework-based applications.
#   Clone from the official repository:
#     https://github.com/FreeRTOS/FreeRTOS-Kernel
#   Set the FREERTOS_KERNEL_PATH environment variable or CMake variable
#   to point to the kernel source directory (not the root of the full FreeRTOS repo).

# - Picotool
#   Used to flash UF2 binaries to the Pico via USB.
#   Will be installed automatically by the Pico SDK. Alternatively, you can download a pre-built binary from:
#   https://github.com/raspberrypi/pico-sdk-tools/releases
#   Must be on your system PATH.

# - OpenOCD (Raspberry Pi fork)
#   Used to flash and debug via SWD using a programmer like Picoprobe.
#   Do not use Homebrew or upstream OpenOCD — they may not support RPxxxx fully.
#   Use the version from: https://github.com/raspberrypi/pico-sdk-tools/releases
#   Must include `scripts/interface/cmsis-dap.cfg` and `scripts/target/rp2040.cfg` / `rp2350.cfg`

# - PicoProbe or other CMSIS-DAP compatible SWD programmer
#   Required for SWD flashing/debugging using OpenOCD.
#   A second Raspberry Pi Pico can be flashed as a Picoprobe.

# - mklittlefs
#   Used to generate LittleFS filesystem images.
#   Required if PICO_HTTP_ENABLE_LITTLEFS is ON.
#   Repository: https://github.com/earlephilhower/mklittlefs
#   Must be built and available on your PATH.

# - pioasm
#   Used to assemble PIO programs. Will be installed automatically by the Pico SDK.
#   If you need a specific version, you can find binaries at:
#   https://github.com/raspberrypi/pico-sdk-tools/releases
#   But pico_sdk_import.cmake and pico_sdk_init() will install it automatically


# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.13)

# Application name (used for project, target, image, etc.)
set(APP_NAME minimum)

# This will set the board and the platform
# Perform a clean build if you change this
set(PICO_BOARD pico_w) # valid: pico_w or pico2_w

if(NOT PICO_BOARD STREQUAL "pico_w" AND NOT PICO_BOARD STREQUAL "pico2_w")
    message(FATAL_ERROR "Unsupported PICO_BOARD: ${PICO_BOARD}")
endif()

# This will set the flash method, which is used to flash the application
# valid: openocd or usb, openocd will be used if you have a picoprobe connected via swd
# picotool will be used if you have a USB connection to the Pico W
set(FLASH_METHOD openocd) # valid: picoprobe or usb

#include the pico sdk
include(../../cmake/pico_sdk_import.cmake)

project(${APP_NAME} CXX C)

# Initialize the pico sdk
pico_sdk_init()

# Size of LittleFS partition (used to calculate offset), unused in this example
# littlefs partition is at the top of flash, so we need to reserve space for it
# offset is calculated as (total flash size - partition size)
set(LFS_PARTITION_SIZE 262144) # 256K

# Feature options - do a clean build if you change these
option(PICO_HTTP_ENABLE_LITTLEFS "Enable LittleFS-based StorageManager" ON)
option(PICO_HTTP_ENABLE_HTTP_CLIENT "Enable HttpClient" ON)
option(PICO_TCP_ENABLE_TLS "Enable TLS support in TCP" ON)
option(PICO_HTTP_ENABLE_JWT "Enable JWT authentication support" OFF)
option(PICO_HTTP_TLS_VERIFY "Enable certificate verification for TLS connections" OFF)

# Credentials (override via env vars if needed)
if(DEFINED ENV{WIFI_SSID})
    set(WIFI_SSID $ENV{WIFI_SSID})
else()
    set(WIFI_SSID "ssid")
endif()

if(DEFINED ENV{WIFI_PASSWORD})
    set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
else()
    set(WIFI_PASSWORD "password")
endif()

# Needed for JWT token generation and authentication, if used
if(DEFINED ENV{JWT_SECRET})
    set(JWT_SECRET_VALUE $ENV{JWT_SECRET})
else()
    set(JWT_SECRET_VALUE "secret")
endif()

# App source files - your list of source files for the application
# This is where you would add your application-specific source files.
set(APP_SOURCES
    src/main.cpp
    src/App.cpp
    src/PingPongController.cpp
)

# Conditionally include the hw_config.c file if building for SD card
if(NOT PICO_HTTP_ENABLE_LITTLEFS) #means that we are using SD card instead of LittleFS
    list(APPEND APP_SOURCES
        src/hw_config.c
    )
endif()

# Shared framework setup, flashing, build logic
# This is designed to remove complexity and duplication from the CMakeLists.txt
# and allow for easy reuse across multiple applications.
# You can use this as a template for your own applications.

include(../../cmake/framework_application.cmake)

# if you need to add additional libraries, you can do so here
# For example, if you have a custom driver library, you can add it like this:
# target_link_libraries(${APP_NAME} my_driver_lib)

# If you have a custom library with headers, you can include it like this:
# target_include_directories(${APP_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/libfoo/include)

# If you need to define any preprocessor definitions, you can do so here
# For example, if you want to enable a specific mode in your application, you can define it like this:
# target_compile_definitions(${APP_NAME} PRIVATE MY_CUSTOM_MODE=1)

# ----------------------------------------------------------------------------------
# Logging and I/O - enable all as we don't know whether the user is using USB or UART
# ----------------------------------------------------------------------------------

pico_enable_stdio_usb(${APP_NAME} 1)
pico_enable_stdio_uart(${APP_NAME} 1)
pico_add_extra_outputs(${APP_NAME})
