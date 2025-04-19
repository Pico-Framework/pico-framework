# ================================================
# PicoFramework - Framework CMakeLists.txt 
# ================================================

cmake_minimum_required(VERSION 3.13)

project(pico_framework)

#------------------------------------------------------------------------------
# 1. Framework Feature Options (only these five)
#------------------------------------------------------------------------------

# Framework Feature Options - if already set by user, use those values
option(PICO_HTTP_ENABLE_LITTLEFS     "Enable LittleFS-based StorageManager" ON)
option(PICO_HTTP_ENABLE_HTTP_CLIENT  "Enable HttpClient" ON)
option(PICO_HTTP_CLIENT_ENABLE_TLS   "Enable TLS support in HttpClient" ON)
option(PICO_HTTP_ENABLE_JWT          "Enable JWT authentication support" ON)
option(PICO_HTTP_TLS_VERIFY          "Enable certificate verification for TLS connections" ON)

#------------------------------------------------------------------------------
# 2. Environment-based Config (Wi-Fi, JWT Secret)
#------------------------------------------------------------------------------

# JWT Secret
if(DEFINED ENV{JWT_SECRET})
    set(JWT_SECRET_VALUE "$ENV{JWT_SECRET}")
else()
    set(JWT_SECRET_VALUE "your_secret")
    message(WARNING "JWT_SECRET not set. Using fallback value.")
endif()
add_definitions(-DJWT_SECRET="${JWT_SECRET_VALUE}")

# Wi-Fi SSID/PASSWORD
if(DEFINED ENV{WIFI_SSID})
    set(WIFI_SSID "$ENV{WIFI_SSID}")
    message(STATUS "Using WIFI_SSID from environment: ${WIFI_SSID}")
endif()
if(DEFINED ENV{WIFI_PASSWORD})
    set(WIFI_PASSWORD "$ENV{WIFI_PASSWORD}")
    message(STATUS "Using WIFI_PASSWORD from environment.")
endif()

#------------------------------------------------------------------------------
# 3. Framework Library and Core Sources
#------------------------------------------------------------------------------

add_library(pico_framework)

target_compile_features(pico_framework PUBLIC cxx_std_17)

target_compile_definitions(pico_framework PUBLIC
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    CYW43_HOST_NAME="Pico-Framework"
    NO_SYS=0
)

# TLS Verify Mode (mbedTLS)
if (PICO_HTTP_TLS_VERIFY)
    add_compile_definitions(ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_REQUIRED)
else()
    add_compile_definitions(ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_NONE)
endif()

# Core sources
target_sources(pico_framework PRIVATE
    # App and core structure
    src/application/AppContext.cpp
    src/application/FrameworkApp.cpp
    src/application/FrameworkController.cpp
    src/application/FrameworkManager.cpp
    src/application/FrameworkTask.cpp
    src/application/FrameworkView.cpp
    src/application/FrameworkModel.cpp

    # HTTP Server
    src/http-server/HttpServer.cpp
    src/http-server/HttpFileserver.cpp
    src/http-server/Middleware.cpp
    src/http-server/Router.cpp
    src/http-server/MultipartParser.cpp

    # HTTP Client
    # is conditionally included below
    
    # HTTP Common
    src/http-common/HttpRequest.cpp
    src/http-common/HttpResponse.cpp
    src/http-common/HttpParser.cpp
    src/http-common/JsonRequestHelper.cpp
    src/http-common/JsonResponse.cpp
    src/http-common/url_utils.cpp

    # Network + Time
    src/network/Network.cpp
    #src/network/NtpClient.cpp
    src/network/Tcp.cpp
    src/network/lwip_dns_resolver.cpp
    src/time/TimeManager.cpp
    src/time/PicoTime.cpp

    # Events
    #src/events/Event.cpp
    src/events/EventManager.cpp
    src/events/GpioEventManager.cpp
    src/events/TimerService.cpp

    # Utility
    src/utility/Logger.cpp
    src/utility/utility.cpp
)

#------------------------------------------------------------------------------
# 4. Conditional Feature Sources
#------------------------------------------------------------------------------

if(PICO_HTTP_ENABLE_JWT)
    target_sources(pico_framework PRIVATE
        src/http-server/JwtAuthenticator.cpp
        src/application/PasswordHasher.cpp
    )
    target_compile_definitions(pico_framework PUBLIC PICO_HTTP_ENABLE_JWT=1)
endif()

# Storage (select one)
if(PICO_HTTP_ENABLE_LITTLEFS)
    message(STATUS "[framework] LittleFS support enabled")
    target_sources(pico_framework PRIVATE
        lib/littlefs/lfs.c
        lib/littlefs/lfs_util.c
        src/storage/LittleFsStorageManager.cpp
        src/port/idleMemory.c
        src/port/cppMemory.cpp
    )
    target_include_directories(pico_framework PUBLIC lib/littlefs)
    target_compile_definitions(pico_framework PUBLIC PICO_HTTP_ENABLE_LITTLEFS=1)
else()
    message(STATUS "[framework] FatFs support enabled")
    target_sources(pico_framework PRIVATE 
        src/storage/FatFsStorageManager.cpp
        src/port/hw_config.c
        src/storage/JsonService.cpp
        )
    target_link_libraries(pico_framework PUBLIC FreeRTOS+FAT+CLI)
endif()

if(PICO_HTTP_ENABLE_HTTP_CLIENT)
    message(STATUS "[framework] HttpClient support enabled")
    target_sources(pico_framework PRIVATE
        src/http-client/HttpClient.cpp
        src/http-client/ChunkedDecoder.cpp
    )
    target_compile_definitions(pico_framework PUBLIC PICO_HTTP_ENABLE_HTTP_CLIENT=1)
endif()

if(PICO_HTTP_CLIENT_ENABLE_TLS)
    message(STATUS "[framework] HttpClient TLS support enabled")
    target_compile_definitions(pico_framework PUBLIC PICO_HTTP_CLIENT_ENABLE_TLS=1)
endif()

#------------------------------------------------------------------------------
# 5. Includes and Libraries
#------------------------------------------------------------------------------

target_include_directories(pico_framework PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/port
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/json/single_include
)

target_link_libraries(pico_framework PUBLIC
    pico_async_context_freertos
    FreeRTOS-Kernel-Heap4
    pico_stdlib
    pico_cyw43_arch_lwip_sys_freertos
    pico_lwip_sntp
    pico_flash
)

if(PICO_PLATFORM STREQUAL "rp2350-arm-s")
    target_link_libraries(pico_framework PUBLIC pico_aon_timer)
endif() 

if(PICO_PLATFORM STREQUAL "rp2040")
    target_link_libraries(pico_framework PUBLIC hardware_rtc)
endif()

if(PICO_HTTP_ENABLE_JWT OR PICO_HTTP_CLIENT_ENABLE_TLS)
    target_link_libraries(pico_framework PUBLIC pico_mbedtls)
endif()

if(PICO_HTTP_CLIENT_ENABLE_TLS)
    target_link_libraries(pico_framework PUBLIC pico_lwip_mbedtls)
endif()

#------------------------------------------------------------------------------
# 6. Warnings / Build Control
#------------------------------------------------------------------------------

target_compile_options(pico_framework PRIVATE -Wno-psabi)

if(PICO_HTTP_CLIENT_ENABLE_TLS)
    set_source_files_properties(
        ${PICO_LWIP_PATH}/src/apps/altcp_tls/altcp_tls_mbedtls.c
        PROPERTIES COMPILE_OPTIONS "-Wno-unused-result"
    )
endif()

#------------------------------------------------------------------------------
# 7. Subdirectories / Aliases
#------------------------------------------------------------------------------

# Set the FreeRTOS config file directory
# Set default FreeRTOS config path if user hasn't provided one
if (NOT DEFINED FREERTOS_CONFIG_FILE_DIRECTORY)
    set(FREERTOS_CONFIG_FILE_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/include/port
        CACHE PATH "Path to FreeRTOSConfig.h used by framework"
    )
    message(STATUS "[framework] Defaulting FREERTOS_CONFIG_FILE_DIRECTORY to: ${FREERTOS_CONFIG_FILE_DIRECTORY}")
else()
    message(STATUS "[framework] Using user-provided FREERTOS_CONFIG_FILE_DIRECTORY: ${FREERTOS_CONFIG_FILE_DIRECTORY}")
endif()

add_subdirectory(lib)
add_library(pico-framework ALIAS pico_framework)
