cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w)

# Pico Framework Feature Options
option(PICO_HTTP_ENABLE_LITTLEFS "Enable LittleFS-based StorageManager" OFF)
option(PICO_HTTP_ENABLE_HTTP_CLIENT "Enable HttpClient" ON)
option(PICO_TCP_ENABLE_TLS "Enable TLS support" ON)
option(PICO_HTTP_ENABLE_JWT "Enable JWT authentication support" ON)
option(PICO_HTTP_TLS_VERIFY "Enable certificate verification for TLS connections" ON)

include(../../cmake/pico_sdk_import.cmake)
include(../../cmake/FreeRTOS_Kernel_import.cmake)

project(Embedded_Tests C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH ${HOME}/.pico/pico-sdk)
endif()

set(FREERTOS_CONFIG_FILE_DIRECTORY ../../framework/include/port)
set(FREERTOS_KERNEL_PATH $ENV{FREERTOS_KERNEL_PATH} CACHE PATH "Path to FreeRTOS kernel")

set(PICO_NO_MBEDTLS TRUE)
set(PICO_HTTP_ENABLE_HTTP_CLIENT ON CACHE BOOL "" FORCE)

pico_sdk_init()

set(PICO_FRAMEWORK_DIR /Users/ianarchbell/Documents/pico-framework)
set(FRAMEWORK_DIR ${PICO_FRAMEWORK_DIR}/framework)
set(FREERTOS_FAT_CLI_DIR ${FRAMEWORK_DIR}/lib/FreeRTOS-FAT-CLI-for-RPi-Pico/src/)
set(TESTS_DIR ${PICO_FRAMEWORK_DIR}/tests)

# Set options BEFORE FetchContent_MakeAvailable
set(CPPUTEST_PLATFORM OFF CACHE BOOL "Providing FreeRTOS platform")
set(CPPUTEST_USE_STD_CPP_LIB ON CACHE BOOL "Using C++ standard library")
set(CPPUTEST_USE_NEW_MACROS 0 CACHE BOOL "Disabling use of new macros")
set(CPPUTEST_USE_LONG_LONG ON CACHE BOOL "Need long long support" FORCE)
set(CPPUTEST_USE_MEM_LEAK_DETECTION OFF CACHE BOOL "Redefined New/Delete mean we have to turn this off" FORCE)
set(TESTS OFF CACHE BOOL "No need to build host tests" FORCE)
set(EXAMPLES OFF CACHE BOOL "No need to build host examples" FORCE)

# === CppUTest Setup ===
include(FetchContent)
FetchContent_Declare(
  CppUTest
  GIT_REPOSITORY https://github.com/cpputest/cpputest.git
  GIT_TAG        v4.0
  SOURCE_SUBDIR  .
  OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(CppUTest)

# Export headers and libs for embedded inclusion
install(TARGETS CppUTest CppUTestExt
    ARCHIVE DESTINATION ${TESTS_DIR}/embedded/lib
)
install(DIRECTORY ${cpputest_SOURCE_DIR}/include/
    DESTINATION ${TESTS_DIR}/embedded/include
)

include_directories(
    ${FRAMEWORK_DIR}/include
    ${FRAMEWORK_DIR}/include/port
    ${cpputest_SOURCE_DIR}/include
)

add_subdirectory(${FRAMEWORK_DIR} pico_framework)

# === Test target ===
add_executable(EmbeddedTest
    #UTestPlatform.cpp
    TestRunner.cpp
    test_StorageManager.cpp
    #test_FatFsStorageManager.cpp
    ${TESTS_DIR}/host/mocks/test_stub_waitpid.c
)

target_include_directories(EmbeddedTest PRIVATE
    ${FRAMEWORK_DIR}/include
    ${FRAMEWORK_DIR}/include/port
    ${FREERTOS_FAT_CLI_DIR}/FreeRTOS+FAT+CLI/include
    ${FREERTOS_FAT_CLI_DIR}/FreeRTOS+FAT+CLI/portable/RP2040
    ${FREERTOS_FAT_CLI_DIR}/Lab-Project-FreeRTOS-FAT/include
    ${cpputest_SOURCE_DIR}/include
    ${cpputest_BINARY_DIR}  # for generated headers like config.h
)

target_compile_definitions(EmbeddedTest PRIVATE
    CPPUTEST_USE_LONG_LONG=1
    CPPUTEST_USE_STD_CPP_LIB=1
    CPPUTEST_MEM_LEAK_DETECTION_DISABLED=1
    CPPUTEST_USE_NEW_MACROS=0
)

target_link_libraries(EmbeddedTest
    pico_framework
    CppUTest
    CppUTestExt
    hardware_spi
)

# Memory map fragment if LittleFS is enabled
if(PICO_HTTP_ENABLE_LITTLEFS)
    target_link_options(EmbeddedTest PRIVATE "-T${CMAKE_SOURCE_DIR}/memmap_lfs_fragment.ld")
else()
    target_sources(EmbeddedTest PRIVATE
        hw_config.c
    )
endif()


target_compile_options(EmbeddedTest PRIVATE
    -Wno-unused-result
    -Wno-psabi
)

pico_enable_stdio_uart(EmbeddedTest 1)
