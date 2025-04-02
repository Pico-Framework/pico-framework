cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico_w) 

#set(PICO_SDK_PATH ~/Documents/Pico/pico-sdk CACHE PATH "Path to the Pico SDK")

include(../../cmake/pico_sdk_import.cmake)  # Or path to your SDK import script

include(../../cmake/FreeRTOS_Kernel_import.cmake)  # Or path to your FreeRTOS import script
# Be able to find FreeRTOSConfig.h
set(FREERTOS_CONFIG_FILE_DIRECTORY ../../framework/include/port)

if (NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message("Exiting as FREERTOS_KERNEL_PATH not defined")
    return()
endif()

project(FatFsStorageManager_Test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED PICO_SDK_PATH)
    set(PICO_SDK_PATH "/Users/ianarchbell/Documents/Pico/pico-sdk")
endif()

# This is to suppress warnings from the nlohmann json library
# The -Wno-psabi flag is used to suppress the "psabi" warning which can occur due to differences in the C++ ABI.
# This is common when using the nlohmann json library, as it can generate warnings about the C++ ABI.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-psabi")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

# Initialize the SDK
pico_sdk_init()

set(FREERTOS_KERNEL_PATH $ENV{FREERTOS_KERNEL_PATH} CACHE PATH "Path to FreeRTOS kernel")

set(PICO_FRAMEWORK_DIR /Users/ianarchbell/Documents/pico-framework) # Ensure this points to the correct pico-framework directory
set(FRAMEWORK_DIR ${PICO_FRAMEWORK_DIR}/framework) # Ensure this points to the correct framework directory
set(TESTS_DIR ${PICO_FRAMEWORK_DIR}/tests) # Ensure this points to the correct tests directory
set(CPPUTEST_DIR ${PICO_FRAMEWORK_DIR}/tests/lib/cpputest) # Ensure this points to the correct tests directory

include_directories(
    ${FRAMEWORK_DIR}/include
    ${FRAMEWORK_DIR}/include/port
    lib 
    test
)

# builds the library
add_subdirectory(${FRAMEWORK_DIR} pico_framework)

add_compile_options(-Wno-psabi)

# only used when running tests on the host
# set(CPPUTEST_ROOT "${CMAKE_CURRENT_LIST_DIR}/../lib/cpputest-install")
# include_directories(${CPPUTEST_ROOT}/include)
# link_directories(${CPPUTEST_ROOT}/lib)

# === Build the test executable ===
add_executable(FatFsStorageManagerTest
    TestRunner.cpp
    FatFsStorageManager_Test.cpp
    JsonService_Test.cpp
    ${TESTS_DIR}/mocks/test_stub_waitpid.c
    ${FRAMEWORK_DIR}/src/port/hw_config.c
)

target_compile_definitions(FatFsStorageManagerTest PRIVATE CPPUTEST_USE_LONG_LONG=1)
target_compile_definitions(FatFsStorageManagerTest PRIVATE PICO_CXX_ENABLE_NEW_DELETE=0)
target_compile_definitions(FatFsStorageManagerTest PRIVATE CPPUTEST_MEM_LEAK_DETECTION=1)
target_compile_definitions(FatFsStorageManagerTest PRIVATE CPPUTEST_USE_STD_CPP_LIB=0)

message("FRAMEWORK_DIR is: ${FRAMEWORK_DIR}")
target_include_directories(FatFsStorageManagerTest PRIVATE 
    ${FRAMEWORK_DIR}/include
    ${FRAMEWORK_DIR}/include/port
    ${CPPUTEST_DIR}/include 
)
message("CPUUTEST_DIR is: ${CPPUTEST_DIR}")
target_link_libraries(FatFsStorageManagerTest
    pico_framework
    ${TESTS_DIR}/lib/build/cpputest/src/CppUtestExt/libCppUTestExt.a
    ${TESTS_DIR}/lib/build/cpputest/src/CppUtest/libCppUTest.a
)

pico_enable_stdio_uart(FatFsStorageManagerTest 1)

