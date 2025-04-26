cmake_minimum_required(VERSION 3.13)

# ---------------------------------------------------------------
# User Configurable Section
# ---------------------------------------------------------------

# Board selection (allow override via environment)
if(DEFINED ENV{PICO_BOARD})
    set(PICO_BOARD $ENV{PICO_BOARD})
else()
    set(PICO_BOARD pico2_w)  # Default board if not provided
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
    set(JWT_SECRET_VALUE "your_secret")  # Default fallback, not secure for production
    message(WARNING "JWT_SECRET environment variable not set. Using fallback value.")
endif()

# Pico Framework Feature Options
option(PICO_HTTP_ENABLE_LITTLEFS "Enable LittleFS-based StorageManager" ON)
option(PICO_HTTP_ENABLE_HTTP_CLIENT "Enable HttpClient" ON)
option(PICO_HTTP_CLIENT_ENABLE_TLS "Enable TLS support in HttpClient" ON)
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
include(../../cmake/pico_sdk_import.cmake)
include(../../cmake/FreeRTOS_Kernel_import.cmake)

# Initialize SDK
pico_sdk_init()

# Set FreeRTOS configuration directory
set(FREERTOS_CONFIG_FILE_DIRECTORY ../../framework/include/port)

# Add Framework
add_subdirectory(../../framework pico_framework)

# ---------------------------------------------------------------
# Application Target Setup
# ---------------------------------------------------------------

project(pico_example_app C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_executable(pico_example_app
    src/main.cpp
    src/App.cpp
    src/StorageController.cpp
    src/FileStorage.cpp
)

# Memory map fragment if LittleFS is enabled
if(PICO_HTTP_ENABLE_LITTLEFS)
    target_link_options(pico_example_app PRIVATE "-T${CMAKE_SOURCE_DIR}/memmap_lfs_fragment.ld")
else()
    target_sources(pico_example_app PRIVATE src/hw_config.c)
endif()

# ---------------------------------------------------------------
# Include Directories
# ---------------------------------------------------------------

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

# ---------------------------------------------------------------
# Linking and Definitions
# ---------------------------------------------------------------

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

# ---------------------------------------------------------------
# Compiler Warnings and Options - ignore nolohmann json warnings and unused result warnings
# ---------------------------------------------------------------

target_compile_options(pico_example_app PRIVATE
    -Wno-unused-result
    -Wno-psabi
)

# ---------------------------------------------------------------
# Output and Console Setup
# ---------------------------------------------------------------

pico_enable_stdio_usb(pico_example_app 0)
pico_enable_stdio_uart(pico_example_app 1)
pico_add_extra_outputs(pico_example_app)
