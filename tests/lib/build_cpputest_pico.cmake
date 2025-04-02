# arm-none-eabi-toolchain.cmake
# set(CMAKE_SYSTEM_NAME Generic)
# set(CMAKE_SYSTEM_PROCESSOR arm)

# set(CMAKE_C_COMPILER arm-none-eabi-gcc)
# set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

# set(CMAKE_C_FLAGS "-mcpu=cortex-m0plus -mthumb -ffunction-sections -fdata-sections")
# set(CMAKE_CXX_FLAGS "-mcpu=cortex-m0plus -mthumb -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections")
# set(CMAKE_EXE_LINKER_FLAGS "-nostartfiles -Wl,--gc-sections")

# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

message("PICO_SDK_PATH: ${PICO_SDK_PATH}")

include(../../cmake/pico_sdk_import.cmake)  # Or path to your SDK import script

pico_sdk_init()

add_definitions(-DCPPUTEST_MEM_LEAK_DETECTION_DISABLED)
