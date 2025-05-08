cmake_minimum_required(VERSION 3.13)
project(mkfs_littlefs_host LANGUAGES C CXX)

# Force this tool to be built for the host, not the embedded target
set(CMAKE_SYSTEM_NAME "")  # Clear cross-compilation settings
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")  # Prevent toolchain reuse

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to LittleFS source
set(LFS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../framework/lib/littlefs")

# Executable for host (macOS/Linux)
add_executable(mkfs_littlefs
    mkfs_littlefs.cpp
    ${LFS_PATH}/lfs.c
    ${LFS_PATH}/lfs_util.c
)

target_include_directories(mkfs_littlefs PRIVATE ${LFS_PATH})

