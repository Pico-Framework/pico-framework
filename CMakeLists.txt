cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# we need this to be able to redefine cpp new/delete with freeRTOS
add_compile_definitions(PICO_CXX_DISABLE_ALLOCATION_OVERRIDES=1) 

# Import SDK
include(${CMAKE_SOURCE_DIR}/cmake/pico_sdk_import.cmake)
pico_sdk_init()

project(framework_test)

# Feature toggles (final apps can set these via -D or at top of this file)
set(PICO_HTTP_ENABLE_LITTLEFS ON)
set(PICO_HTTP_ENABLE_HTTP_CLIENT ON)
set(PICO_HTTP_CLIENT_ENABLE_TLS ON)
set(PICO_HTTP_ENABLE_JWT ON)
set(PICO_HTTP_TLS_VERIFY ON)

include(${CMAKE_SOURCE_DIR}/cmake/FreeRTOS_Kernel_import.cmake)

if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message("Exiting as FREERTOS_KERNEL_PATH not defined")
    return()
endif()

# Include wrapper (sets up SDK, board, linker, framework)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/pico_framework_app.cmake)

# Declare the test app
pico_framework_app(
    NAME framework_test
    SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/framework/src/main.cpp
)
