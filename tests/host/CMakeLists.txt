cmake_minimum_required(VERSION 3.13)

# Force host compiler (Mac / Linux)
set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)

project(EmbeddedHttpTests C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

set (FRAMEWORK_DIR /Users/ianarchbell/Documents/pico-framework/framework)

# Add CppUTest
include(FetchContent)

FetchContent_Declare(
  CppUTest
  GIT_REPOSITORY https://github.com/cpputest/cpputest.git
  GIT_TAG        v4.0
)

# Optional: Skip internal CppUTest test builds
set(TESTS OFF CACHE BOOL "Disable CppUTest tests" FORCE)
set(EXAMPLES OFF CACHE BOOL "Disable CppUTest examples" FORCE)
set(CPPUTEST_USE_LONG_LONG 1 CACHE BOOL "Enable long long support" FORCE)

FetchContent_MakeAvailable(CppUTest)

# Clean memory leak setup for host: ENABLED
target_compile_definitions(CppUTest PUBLIC
    CPPUTEST_USE_MEM_LEAK_DETECTION=1
)

target_compile_definitions(CppUTestExt PUBLIC
    CPPUTEST_USE_MEM_LEAK_DETECTION=1
)

message("FRAMEWRORK_DIR: ${FRAMEWORK_DIR}")

# Include the framework headers
include_directories(
    ${FRAMEWORK_DIR}/include
    ${FRAMEWORK_DIR}/lib/json/single_include
    ./mocks
    ./mocks/lwip
    )

# Your main code
add_library(http_core
    ${FRAMEWORK_DIR}/include/framework_config.h
    ${FRAMEWORK_DIR}/src/http/HttpRequest.cpp
    ${FRAMEWORK_DIR}/include/HttpRequest.h
    ${FRAMEWORK_DIR}/include/HttpResponse.h
    ${FRAMEWORK_DIR}/src/http/HttpResponse.cpp
    ${FRAMEWORK_DIR}/src/http/MultipartParser.cpp
    ${FRAMEWORK_DIR}/src/utility/utility.cpp 
    ${FRAMEWORK_DIR}/include/utility.h
    ${FRAMEWORK_DIR}/src/application/AppContext.cpp 
    ${FRAMEWORK_DIR}/include/AppContext.h
    ${FRAMEWORK_DIR}/src/storage/FatFsStorageManager.cpp
    ${FRAMEWORK_DIR}/include/FatFsStorageManager.h
    ${FRAMEWORK_DIR}/src/http/url_utils.cpp
    ${FRAMEWORK_DIR}/include/url_utils.h
    ${FRAMEWORK_DIR}/src/http/HttpResponse.cpp
    ${FRAMEWORK_DIR}/include/HttpResponse.h
    # other files
)

target_compile_definitions(http_core PRIVATE UNIT_TEST)

# Your tests
# Add the test executable
add_executable(HttpRequestTest
    HttpRequest_Test.cpp
    AllTests.cpp
    )

add_executable(HttpResponseTest    
    HttpResponse_Test.cpp
    AllTests.cpp        
    ) 

target_compile_definitions(HttpRequestTest PRIVATE UNIT_TEST)

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-wrap,lwip_send")
endif()

# Link test executable to your code + CppUTest
target_link_libraries(HttpRequestTest
    http_core
    CppUTest
    CppUTestExt
)

target_link_libraries(HttpResponseTest
    http_core
    CppUTest
    CppUTestExt
)
