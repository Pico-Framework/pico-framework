cmake_minimum_required(VERSION 3.13)

if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
    set(WIFI_SSID $ENV{WIFI_SSID})
    message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
endif()
if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
    set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
    message("Using WIFI_PASSWORD from environment")
endif()

set(PICO_BOARD pico_w) 

include(../../cmake/pico_sdk_import.cmake)  # Or path to your SDK import script

include(../../cmake/FreeRTOS_Kernel_import.cmake)  # Or path to your FreeRTOS import script
# Be able to find FreeRTOSConfig.h
set(FREERTOS_CONFIG_FILE_DIRECTORY ../../framework/include/port)

if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message("Exiting as FREERTOS_KERNEL_PATH not defined")
    return()
endif()

project(pico_example_app C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add your main app source files
add_executable(pico_example_app
    src/main.cpp
    src/app.cpp
    src/hw_config.c
)

# Include paths for headers (adjust as needed)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../framework/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../framework/include/port
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../framework/lib/FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../framework/lib/FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/portable/RP2040
    ${CMAKE_CURRENT_SOURCE_DIR}/../../framework/lib/FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/include 
)

set(FREERTOS_KERNEL_PATH $ENV{FREERTOS_KERNEL_PATH} CACHE PATH "Path to FreeRTOS kernel")

# builds the library
add_subdirectory(../../framework pico_framework)

# Link against the separately-built framework library
target_link_libraries(pico_example_app
    pico_stdlib
    pico_framework
)

target_compile_definitions(pico_example_app PUBLIC 
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    CYW43_HOST_NAME="Pico-Framework"
    ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_REQUIRED
    NO_SYS=0
) 

# Read the environment variable at configure time
if(DEFINED ENV{JWT_SECRET})
    set(JWT_SECRET_VALUE "$ENV{JWT_SECRET}")
else()
    set(JWT_SECRET_VALUE "your_secret")  # Fallback value - shouldn't do this in production
    message(WARNING "JWT_SECRET environment variable not set. Using fallback value.")
    # You might want to handle this differently in production
    # e.g., by failing the configuration or logging an error.
endif()

# Optional: enable USB or UART if needed
pico_enable_stdio_usb(pico_example_app 0)
pico_enable_stdio_uart(pico_example_app 1)

# Create .uf2 file
pico_add_extra_outputs(pico_example_app)
