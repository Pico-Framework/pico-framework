cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD pico_w)

include(${CMAKE_SOURCE_DIR}/cmake/pico_sdk_import.cmake)

project(pico-sprinkler C CXX ASM)

include(${CMAKE_SOURCE_DIR}/cmake/FreeRTOS_Kernel_import.cmake)

if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message("Exiting as FREERTOS_KERNEL_PATH not defined")
    return()
endif()

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

#quiet down warnings on nlohmann json library
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-psabi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

# Initialize the SDK
pico_sdk_init()

# global include because pico sdk has includes that will fail if not included
# we do not link it in unless option is set beow
include_directories(${PICO_SDK_PATH}/lib/mbedtls/include)

option(PICO_HTTP_ENABLE_LITTLEFS "Enable LittleFS-based StorageManager" ON)
option(PICO_HTTP_ENABLE_HTTP_CLIENT "Enable HttpClient" ON)
option(PICO_HTTP_CLIENT_ENABLE_TLS "Enable TLS support in HttpClient" ON)
option(PICO_HTTP_ENABLE_JWT "Enable JWT authentication support" ON)
option(PICO_HTTP_TLS_VERIFY "Enable certificate verification for TLS connections" ON)

# Add subdirectories
add_subdirectory(framework)

# Dummy executable to measure memory usage of the framework
add_executable(framework_test framework/src/main.cpp)
target_link_libraries(framework_test pico_framework)

#get the memory usage
if (PICO_PRINT_MEM_USAGE)

    # Ensure CMAKE_SIZE and CMAKE_OBJDUMP are set
    find_program(CMAKE_SIZE arm-none-eabi-size)
    find_program(CMAKE_OBJDUMP arm-none-eabi-objdump)

    if (NOT CMAKE_SIZE)
        message(FATAL_ERROR "CMAKE_SIZE (arm-none-eabi-size) not found")
    endif()
    if (NOT CMAKE_OBJDUMP)
        message(FATAL_ERROR "CMAKE_OBJDUMP (arm-none-eabi-objdump) not found")
    endif()

    # Create post-build diagnostics as a separate target
    add_custom_target(report_memory ALL
        DEPENDS framework_test
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:framework_test>
        COMMAND ${CMAKE_OBJDUMP} -h $<TARGET_FILE:framework_test>
        COMMAND ${CMAKE_COMMAND}
            -DPATH_TO_ELF=$<TARGET_FILE:framework_test>
            -DCMAKE_OBJDUMP=${CMAKE_OBJDUMP}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tools/report_memory.cmake
        COMMENT "📊 Running Framework RAM usage analysis..."
    )
endif()