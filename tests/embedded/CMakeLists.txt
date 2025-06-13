cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w)

# Pico Framework Feature Options
option(PICO_HTTP_ENABLE_LITTLEFS "Enable LittleFS-based StorageManager" ON)
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

# enabling exceptions for tests
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

pico_sdk_init()

# Try to set PICO_FRAMEWORK_DIR from environment, or default to relative
if(NOT DEFINED PICO_FRAMEWORK_DIR)
    if(DEFINED ENV{PICO_FRAMEWORK_DIR})
        set(PICO_FRAMEWORK_DIR $ENV{PICO_FRAMEWORK_DIR})
    else()
        set(PICO_FRAMEWORK_DIR ${CMAKE_CURRENT_LIST_DIR}/../..) 
    endif()
endif()

set(FRAMEWORK_DIR ${PICO_FRAMEWORK_DIR}/framework)
set(FREERTOS_FAT_CLI_DIR ${FRAMEWORK_DIR}/lib/FreeRTOS-FAT-CLI-for-RPi-Pico/src/)
set(TESTS_DIR ${PICO_FRAMEWORK_DIR}/tests)

# === CppUTest Setup ===
include(FetchContent)
FetchContent_Declare(
  CppUTest
  GIT_REPOSITORY https://github.com/cpputest/cpputest.git
  GIT_TAG        v4.0
)

# Set options BEFORE FetchContent_MakeAvailable
option(MEMORY_LEAK_DETECTION "Enable memory leak detection" OFF) # this works!
option(LONGLONG "Support long long" ON)
option(STD_CPP "Use the standard C++ library" ON)
option(TESTS "Compile and make tests for the code?" OFF)
option(EXAMPLES "Compile and make examples?" OFF)
option(C++11 "Compile with C++11 support" ON)

#set(CPPUTEST_USE_LONG_LONG 1 CACHE BOOL "Use long long")

#set(CPPUTEST_PLATFORM OFF CACHE BOOL "Providing FreeRTOS platform")

add_compile_definitions(EmbeddedTest PRIVATE
    PICO_CXX_DISABLE_ALLOCATION_OVERRIDES
    CPPUTEST_USE_MEM_LEAK_DETECTION=0
    CPPUTEST_USE_LONG_LONG=1
)

FetchContent_MakeAvailable(CppUTest)

# === Test target ===
add_executable(EmbeddedTest
    UTestPlatform.cpp
    TestRunner.cpp
    test_StorageManager.cpp
    ${TESTS_DIR}/host/mocks/test_stub_waitpid.c
)

unset(CPPUTEST_USE_NEW_MACROS)

# After FetchContent_MakeAvailable(CppUTest)
add_library(CppUTest::CppUTest ALIAS CppUTest)
add_library(CppUTest::CppUTestExt ALIAS CppUTestExt)

target_link_libraries(EmbeddedTest PRIVATE
    pico_framework
    CppUTest::CppUTest
    CppUTest::CppUTestExt
    hardware_spi
)

include(CTest) # Enable testing

# automatically discover CppUTest tests
include(${CMAKE_CURRENT_SOURCE_DIR}/lib/cpputest/cmake/Modules/CppUTest.cmake)
cpputest_discover_tests(EmbeddedTest)

include_directories(
    ${FRAMEWORK_DIR}/include
    ${FRAMEWORK_DIR}/include/port
    ${cpputest_SOURCE_DIR}/include
)

add_subdirectory(${FRAMEWORK_DIR} pico_framework)


target_include_directories(EmbeddedTest PRIVATE
    ${FRAMEWORK_DIR}/include
    ${FRAMEWORK_DIR}/include/port
    ${FREERTOS_FAT_CLI_DIR}/FreeRTOS+FAT+CLI/include
    ${FREERTOS_FAT_CLI_DIR}/FreeRTOS+FAT+CLI/portable/RP2040
    ${FREERTOS_FAT_CLI_DIR}/Lab-Project-FreeRTOS-FAT/include
    ${cpputest_SOURCE_DIR}/include
    ${cpputest_BINARY_DIR}  # for generated headers like config.h
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
