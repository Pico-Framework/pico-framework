cmake_minimum_required(VERSION 3.13)

# Import Pico SDK
include(../../cmake/pico_sdk_import.cmake)  # Adjust if needed

# Set project
project(CppUTest_ARM)

pico_sdk_init()

# Only build the CppUTest library, skip its own tests/examples
set(TESTS OFF CACHE BOOL "Disable building CppUTest tests")
set(EXAMPLES OFF CACHE BOOL "Disable building CppUTest examples")

# Enable long long support in CppUTest library itself
set(CPPUTEST_USE_LONG_LONG 1 CACHE BOOL "Enable long long support in CppUTest")

add_subdirectory(CppUTest)

# Force long long support and disable CppUTest's operator new/delete
target_compile_definitions(CppUTest PUBLIC
    CPPUTEST_USE_LONG_LONG=1
    CPPUTEST_USE_MEM_LEAK_DETECTION=0
    FatFsStorageManagerTest PRIVATE CPPUTEST_USE_STD_CPP_LIB=0
)
target_compile_definitions(CppUTestExt PUBLIC
    CPPUTEST_USE_LONG_LONG=1
    CPPUTEST_USE_MEM_LEAK_DETECTION=0
    FatFsStorageManagerTest PRIVATE CPPUTEST_USE_STD_CPP_LIB=0
)
