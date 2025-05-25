cmake_minimum_required(VERSION 3.13)

# Application name (used for project, target, image, etc.)
set(APP_NAME minimum)

# This will set the board and the platform
set(PICO_BOARD pico2_w) # valid: pico_w or pico2_w

# This will set the flash method, which is used to flash the application
# valid: picoprobe or usb, openocd will be used if you have a picoprobe connected via swd
# picotool will be used if you have a USB connection to the Pico W
set(FLASH_METHOD usb)   # valid: picoprobe or usb

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
option(PICO_HTTP_ENABLE_LITTLEFS "Enable LittleFS-based StorageManager" OFF)
option(PICO_HTTP_ENABLE_HTTP_CLIENT "Enable HttpClient" OFF)
option(PICO_TCP_ENABLE_TLS "Enable TLS support in TCP" OFF)
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

# Needed for JWT authentication
if(DEFINED ENV{JWT_SECRET})
    set(JWT_SECRET_VALUE $ENV{JWT_SECRET})
else()
    set(JWT_SECRET_VALUE "secret")
endif()

# App source files
set(APP_SOURCES
    src/main.cpp
    src/app.cpp
    src/hw_config.c
)

# Shared framework setup, flashing, build logic
# Thisis designed to remove complexity and duplication from the CMakeLists.txt
# and allow for easy reuse across multiple applications.
# You can use this as a template for your own applications.
# For example adding other link libraries, compiler flags or defintions etc.
include(../../cmake/framework_application.cmake)

# ----------------------------------------------------------------------------------
# Logging and I/O - enable all as we don't know whether the user is using USB or UART
# ----------------------------------------------------------------------------------

pico_enable_stdio_usb(${APP_NAME} 1)
pico_enable_stdio_uart(${APP_NAME} 1)
pico_add_extra_outputs(${APP_NAME})
