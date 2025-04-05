cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w) 

include(../../cmake/pico_sdk_import.cmake)
include(../../cmake/FreeRTOS_Kernel_import.cmake)

set(FREERTOS_CONFIG_FILE_DIRECTORY ../../framework/include/port)

if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message("Exiting as FREERTOS_KERNEL_PATH not defined")
    return()
endif()

project(Embedded_Tests C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH "/Users/ianarchbell/Documents/Pico/pico-sdk")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-psabi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

set(PICO_NO_MBEDTLS TRUE)

set(PICO_HTTP_ENABLE_HTTP_CLIENT ON CACHE BOOL "" FORCE)

pico_sdk_init()

set(FREERTOS_KERNEL_PATH $ENV{FREERTOS_KERNEL_PATH} CACHE PATH "Path to FreeRTOS kernel")
set(PICO_FRAMEWORK_DIR /Users/ianarchbell/Documents/pico-framework)
set(FRAMEWORK_DIR ${PICO_FRAMEWORK_DIR}/framework)
set(TESTS_DIR ${PICO_FRAMEWORK_DIR}/tests)

set(CPPUTEST_USE_MEM_LEAK_DETECTION 0 CACHE BOOL "Disable new/delete for embedded" FORCE)
set(CPPUTEST_USE_NEW_MACROS 0 CACHE BOOL "Disable macro new for embedded" FORCE)
set(CPPUTEST_USE_STD_CPP_LIB 0 CACHE BOOL "Avoid std C++ lib on embedded" FORCE)

# === Add CppUTest via FetchContent ===
include(FetchContent)

FetchContent_Declare(
  CppUTest
  GIT_REPOSITORY https://github.com/cpputest/cpputest.git
  GIT_TAG        v4.0
  OVERRIDE_FIND_PACKAGE
  SOURCE_SUBDIR .
)

# Set options **before** making available
set(TESTS OFF CACHE BOOL "Disable CppUTest tests" FORCE)
set(EXAMPLES OFF CACHE BOOL "Disable CppUTest examples" FORCE)
set(CPPUTEST_USE_MEM_LEAK_DETECTION 0 CACHE BOOL "" FORCE)
set(CPPUTEST_USE_NEW_MACROS 0 CACHE BOOL "" FORCE)
set(CPPUTEST_USE_STD_CPP_LIB 0 CACHE BOOL "" FORCE)
set(CPPUTEST_USE_LONG_LONG 1 CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(CppUTest)

# Configure the actual targets
target_compile_definitions(CppUTest PUBLIC
    CPPUTEST_USE_MEM_LEAK_DETECTION=0
    CPPUTEST_USE_STD_CPP_LIB=0
)
target_compile_definitions(CppUTestExt PUBLIC
    CPPUTEST_USE_MEM_LEAK_DETECTION=0
    CPPUTEST_USE_STD_CPP_LIB=0
)

# Optional: Disable building CppUTest examples/tests and enable long long
set(TESTS OFF CACHE BOOL "Disable CppUTest tests" FORCE)
set(EXAMPLES OFF CACHE BOOL "Disable CppUTest examples" FORCE)
set(CPPUTEST_USE_LONG_LONG 1 CACHE BOOL "Enable long long support" FORCE)

FetchContent_MakeAvailable(CppUTest)

# Local install for embedded linking
install(TARGETS CppUTest CppUTestExt
    ARCHIVE DESTINATION ${TESTS_DIR}/embedded/lib
)

install(DIRECTORY ${cpputest_SOURCE_DIR}/include/
    DESTINATION ${TESTS_DIR}/embedded/include
)

include_directories(
    ${FRAMEWORK_DIR}/include
    ${FRAMEWORK_DIR}/include/port
    ${TESTS_DIR}/embedded/include
)

add_subdirectory(${FRAMEWORK_DIR} pico_framework)
add_compile_options(-Wno-psabi)

# === Test target ===
add_executable(EmbeddedTest
    TestRunner.cpp
    #FatFsStorageManager_Test.cpp
    #JsonService_Test.cpp
    #PasswordHash_test.cpp
    HttpClient_Test.cpp
    ${TESTS_DIR}/host/mocks/test_stub_waitpid.c
    ${FRAMEWORK_DIR}/src/port/hw_config.c
)

target_compile_definitions(EmbeddedTest PRIVATE
    CPPUTEST_USE_LONG_LONG=1
    PICO_CXX_ENABLE_NEW_DELETE=0
    CPPUTEST_MEM_LEAK_DETECTION=1
    CPPUTEST_USE_STD_CPP_LIB=0
)

message("FRAMEWORK_DIR is: ${FRAMEWORK_DIR}")
message("TESTS_DIR is: ${TESTS_DIR}")

target_include_directories(EmbeddedTest PRIVATE 
    ${FRAMEWORK_DIR}/include
    ${FRAMEWORK_DIR}/include/port
    ${cpputest_SOURCE_DIR}/include
    ${cpputest_BINARY_DIR}  # Needed for generated headers like config.h
)

target_link_libraries(EmbeddedTest
    pico_framework
    CppUTest
    CppUTestExt
)

pico_enable_stdio_uart(EmbeddedTest 1)
