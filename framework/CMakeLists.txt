if (DEFINED ENV{WIFI_SSID} AND (NOT WIFI_SSID))
    set(WIFI_SSID $ENV{WIFI_SSID})
    message("Using WIFI_SSID from environment ('${WIFI_SSID}')")
endif()
if (DEFINED ENV{WIFI_PASSWORD} AND (NOT WIFI_PASSWORD))
    set(WIFI_PASSWORD $ENV{WIFI_PASSWORD})
    message("Using WIFI_PASSWORD from environment")
endif()

# Framework library
add_library(pico_framework)

# Set C++ standard
target_compile_features(pico_framework PUBLIC cxx_std_17)

target_compile_definitions(pico_framework PUBLIC
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

# Pass the secret as a macro definition
add_definitions(-DJWT_SECRET="${JWT_SECRET_VALUE}")

# Source files (organized by domain)
target_sources(pico_framework
    PRIVATE
        src/application/FrameworkManager.cpp 
        src/application/FrameworkTask.cpp
        src/application/FrameworkApp.cpp
        src/application/AppContext.cpp
        src/Application/FrameworkController.cpp
        src/Application/FrameworkModel.cpp
        src/application/FrameworkView.cpp
        src/http/HttpServer.cpp
        src/http/HttpRequest.cpp
        src/http/HttpResponse.cpp
        src/http/HttpFileserver.cpp
        src/http/JwtAuthenticator.cpp
        src/http/JsonRequestHelper.cpp # (not mandatory, e.g. auto name = JsonRequestHelper::getString(req, "user.name");)
        # or if (JSON::hasField(req, "auth.token")) { ... } JSON is a macro for JsonRequestHelper
        # Only if you want to use JSON request helper methods
        src/http/Middleware.cpp
        src/http/Router.cpp
        src/http/MultipartParser.cpp
        src/http/url_utils.cpp
        #src/network/TcpConnectionSocket.cpp abstraction coming to use your own layer (e.g. raw ethernet, lwip, etc.)
        src/network/Network.cpp 
        src/network/NtpClient.cpp
        src/events/EventManager.cpp
        src/events/TimerService.cpp
        src/utility/Logger.cpp
        src/utility/utility.cpp
        src/port/hw_config.c
        src/storage/FatFsStorageManager.cpp
        src/storage/JsonService.cpp
        src/time/PicoTime.cpp
)

target_link_libraries(pico_framework PUBLIC
        pico_async_context_freertos
        FreeRTOS-Kernel-Heap4
        pico_stdlib
        hardware_i2c
        hardware_pwm
        hardware_spi
        pico_cyw43_arch_lwip_sys_freertos
        hardware_rtc
        FreeRTOS+FAT+CLI
        pico_lwip_mbedtls
        pico_lwip_http
        pico_mbedtls
    )    

# Public headers (flat include path for now)
target_include_directories(pico_framework
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/port
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/json/single_include
        ${FREERTOS_KERNEL_PATH}/include
        ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/portable/RP2040
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/include
)

target_compile_options(pico_framework PRIVATE -Wno-psabi)

# Ignore warnings from lwip code
set_source_files_properties(
        ${PICO_LWIP_PATH}/src/apps/altcp_tls/altcp_tls_mbedtls.c
        PROPERTIES
        COMPILE_OPTIONS "-Wno-unused-result" 
        )

# These are needed to avoid warnings from the FreeRTOS+FAT code
include_directories(
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include
)

# Set the FreeRTOS config file directory
if (NOT DEFINED FREERTOS_CONFIG_FILE_DIRECTORY)
    set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
endif()

add_subdirectory(lib)



