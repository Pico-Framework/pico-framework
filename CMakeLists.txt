cmake_minimum_required(VERSION 3.13)

# ---------------------------------------------------------------
# User Configurable Section
# ---------------------------------------------------------------

# Board selection (allow override via environment)
# if(DEFINED ENV{PICO_BOARD})
#     set(PICO_BOARD $ENV{PICO_BOARD})
# else()
    set(PICO_BOARD pico_w)
#endif()

# Pico Framework Feature Options
option(PICO_HTTP_ENABLE_LITTLEFS "Enable LittleFS-based StorageManager" OFF)
option(PICO_HTTP_ENABLE_HTTP_CLIENT "Enable HttpClient" ON)
option(PICO_TCP_ENABLE_TLS "Enable TLS support" ON)
option(PICO_HTTP_ENABLE_JWT "Enable JWT authentication support" ON)
option(PICO_HTTP_TLS_VERIFY "Enable certificate verification for TLS connections" ON)

# ---------------------------------------------------------------
# SDK and Framework Setup
# ---------------------------------------------------------------

# Check for Pico SDK path
if(NOT PICO_SDK_PATH AND NOT DEFINED ENV{PICO_SDK_PATH})
    message(FATAL_ERROR "PICO_SDK_PATH is not set. Please define it in the environment or CMake cache.")
endif()

# Check for FreeRTOS path
if(NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message(FATAL_ERROR "FREERTOS_KERNEL_PATH is not set. Please define it in the environment or CMake cache.")
endif()

# Import Pico SDK and FreeRTOS
include(${CMAKE_SOURCE_DIR}/cmake/pico_sdk_import.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/FreeRTOS_Kernel_import.cmake)

# SDK version check
if (PICO_SDK_VERSION_STRING VERSION_LESS "2.1.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.1.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize SDK
pico_sdk_init()

# Global include for mbedtls headers (required even if HttpClient is optional)
include_directories(${PICO_SDK_PATH}/lib/mbedtls/include)

# Add Framework
add_subdirectory(framework)

# ---------------------------------------------------------------
# Application Target Setup
# ---------------------------------------------------------------

project(framework_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(framework_test
    framework/src/test-app/main.cpp
    framework/src/test-app/app.cpp
    framework/src/test-app/DashboardController.cpp
    framework/src/test-app/GpioController.cpp
    framework/src/test-app/PicoModel.cpp
    framework/src/test-app/DashboardView.cpp
)

# Memory map fragment if LittleFS is enabled
if(PICO_HTTP_ENABLE_LITTLEFS)
    target_link_options(framework_test PRIVATE "-T${CMAKE_SOURCE_DIR}/memmap_lfs_fragment.ld")
else()
    target_sources(framework_test PRIVATE
        framework/src/test-app/hw_config.c
    )
endif()

# ---------------------------------------------------------------
# Include Directories
# ---------------------------------------------------------------

target_include_directories(framework_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/src/test-app/include
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/include
    ${CMAKE_CURRENT_SOURCE_DIR}/framework/include/port
)

# Board-specific FreeRTOS port
if(${PICO_BOARD} STREQUAL "pico2_w")
    target_include_directories(framework_test PRIVATE ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure/include)
else()
    target_include_directories(framework_test PRIVATE ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include)
endif()

# ---------------------------------------------------------------
# Linking and Definitions
# ---------------------------------------------------------------

target_link_libraries(framework_test
    pico_framework
    pico_stdlib
    hardware_adc
)

target_compile_definitions(framework_test PUBLIC
    PICO_CXX_DISABLE_ALLOCATION_OVERRIDES=1
)

add_compile_definitions(PICO_MAX_SHARED_IRQ_HANDLERS=8)

# ---------------------------------------------------------------
# Compiler Warnings and Options
# ---------------------------------------------------------------

target_compile_options(framework_test PRIVATE
    -Wno-unused-result
    -Wno-psabi
)

# ---------------------------------------------------------------
# Output and Console Setup
# ---------------------------------------------------------------

pico_enable_stdio_usb(framework_test 1)
pico_enable_stdio_uart(framework_test 1)
pico_add_extra_outputs(framework_test)

# ---------------------------------------------------------------
# Post-Build Diagnostics (Optional Memory Report)
# ---------------------------------------------------------------

option(PICO_PRINT_MEM_USAGE "Enable memory usage report after build" OFF)

find_program(CMAKE_SIZE arm-none-eabi-size)
if (NOT CMAKE_SIZE)
    message(FATAL_ERROR "CMAKE_SIZE (arm-none-eabi-size) not found")
endif()

add_custom_command(
        TARGET framework_test
        POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:framework_test>
        COMMENT "Printing memory usage for framework_test"
    )

if(PICO_PRINT_MEM_USAGE STREQUAL "ON")
    message(STATUS "Memory usage report enabled")
    find_program(CMAKE_OBJDUMP arm-none-eabi-objdump)
    if (NOT CMAKE_OBJDUMP)
        message(FATAL_ERROR "CMAKE_OBJDUMP (arm-none-eabi-objdump) not found")
    endif()

    add_custom_target(report_memory ALL
        DEPENDS framework_test

        COMMAND ${CMAKE_OBJDUMP} -h $<TARGET_FILE:framework_test>
        COMMAND ${CMAKE_COMMAND}
            -DPATH_TO_ELF=$<TARGET_FILE:framework_test>
            -DCMAKE_OBJDUMP=${CMAKE_OBJDUMP}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tools/report_memory.cmake
        COMMENT "Running Framework RAM usage analysis..."
    )
endif()

add_custom_target(flash
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/picotool load test-app.uf2 -f
    DEPENDS test-app.uf2
    COMMENT "Flashing test-app.uf2 using picotool"
)
