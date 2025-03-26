# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD pico_w)

include(${CMAKE_SOURCE_DIR}/cmake/pico_sdk_import.cmake)

project(pico-sprinkler C CXX ASM)

include(${CMAKE_SOURCE_DIR}/cmake/FreeRTOS_Kernel_import.cmake)

if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message("Exiting as FREERTOS_KERNEL_PATH not defined")
    return()
endif()

# Be able to find FreeRTOSConfig.h
set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# we need this to be able to redefine cpp new/delete with freeRTOS
add_compile_definitions(PICO_CXX_DISABLE_ALLOCATION_OVERRIDES=1) 

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.1.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.1.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
	return()
endif()

# If you want debug output from USB (pass -DPICO_STDIO_USB=1) this ensures you don't lose any debug output while USB is set up
if (NOT DEFINED PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS)
    set(PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 3000)
endif()

# Initialize the SDK

pico_sdk_init()

# Add subdirectories
add_subdirectory(framework)
