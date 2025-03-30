cmake_minimum_required(VERSION 3.13)

# Force host compiler (Mac / Linux)
set(CMAKE_SYSTEM_NAME Darwin)
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)

project(EmbeddedHttpTests C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add CppUTest
add_subdirectory(lib/cpputest)

# Include the framework headers
include_directories(
    ../framework/include
    ./mocks
    )


# Your main code
add_library(http_core
    ../framework/src/http/HttpRequest.cpp
    ../framework/include/HttpRequest.h
    ../framework/src/http/MultipartParser.cpp
    ../framework/src/utility/utility.cpp 
    ../framework/include/utility.h
    ../framework/src/application/AppContext.cpp 
    ../framework/include/AppContext.h
    ../framework/src/storage/FatFsStorageManager.cpp
    ../framework/include/FatFsStorageManager.h
    ../framework/src/http/url_utils.cpp
    ../framework/include/url_utils.h
    ../framework/src/http/HttpResponse.cpp
    ../framework/include/HttpResponse.h
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

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-wrap,lwip_send")

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