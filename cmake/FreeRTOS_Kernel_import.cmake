# @file FreeRTOS_Kernel_import.cmake
# @brief Clean, portable FreeRTOS kernel import for RP2040/RP2350
# @note Only uses FREERTOS_KERNEL_PATH and PICO_PLATFORM

# --- Get FREERTOS_KERNEL_PATH from environment if not already set ---
if (DEFINED ENV{FREERTOS_KERNEL_PATH} AND NOT FREERTOS_KERNEL_PATH)
    set(FREERTOS_KERNEL_PATH $ENV{FREERTOS_KERNEL_PATH})
    message(STATUS "Using FREERTOS_KERNEL_PATH from environment: ${FREERTOS_KERNEL_PATH}")
endif()

# --- Fail early if it's still missing ---
if (NOT FREERTOS_KERNEL_PATH)
    message(FATAL_ERROR "FREERTOS_KERNEL_PATH is not set. Define it in CMake or as an environment variable.")
endif()

# --- Set default platform if missing (for standalone testing) ---
if (NOT DEFINED PICO_PLATFORM)
    set(PICO_PLATFORM "rp2040")
    message(WARNING "PICO_PLATFORM not set. Defaulting to 'rp2040'")
endif()

# --- Choose FreeRTOS port directory based on PICO_PLATFORM ---
if (PICO_PLATFORM STREQUAL "rp2040")
    set(FREERTOS_KERNEL_PORT_NAME "RP2040")
    set(FREERTOS_KERNEL_PORT_PATH "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/${FREERTOS_KERNEL_PORT_NAME}")
elseif (PICO_PLATFORM STREQUAL "rp2350-riscv")
    set(FREERTOS_KERNEL_PORT_NAME "RP2350_RISC-V")
    set(FREERTOS_KERNEL_PORT_PATH "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/${FREERTOS_KERNEL_PORT_NAME}")
elseif (PICO_PLATFORM STREQUAL "rp2350-arm-s")
    set(FREERTOS_KERNEL_PORT_NAME "RP2350_ARM_NTZ")
    set(FREERTOS_KERNEL_PORT_PATH "${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/${FREERTOS_KERNEL_PORT_NAME}")
else()
    message(FATAL_ERROR "Unknown PICO_PLATFORM: ${PICO_PLATFORM}")
endif()

message(STATUS "FREERTOS kernel port selected: ${FREERTOS_KERNEL_PORT_NAME}")
message(STATUS "FREERTOS kernel port path: ${FREERTOS_KERNEL_PORT_PATH}")

# --- Verify the path and bring in the port ---
if (NOT EXISTS "${FREERTOS_KERNEL_PORT_PATH}/CMakeLists.txt")
    message(FATAL_ERROR "Expected port CMakeLists.txt not found at: ${FREERTOS_KERNEL_PORT_PATH}")
endif()

message(STATUS "Adding FreeRTOS kernel port from: ${FREERTOS_KERNEL_PORT_PATH}")
# Add the actual port as a subdirectory
add_subdirectory(${FREERTOS_KERNEL_PORT_PATH} FREERTOS_KERNEL)
