cmake_minimum_required(VERSION 3.13)
project(FreeRTOS_FAT_CLI LANGUAGES C CXX)

# Required: path to FreeRTOS-Kernel
if(NOT DEFINED FREERTOS_KERNEL_PATH)
    message(FATAL_ERROR "[FAT+CLI] FREERTOS_KERNEL_PATH is not defined. Pass it via -DFREERTOS_KERNEL_PATH=<...>")
endif()

# Optional override: path to FreeRTOS-FAT-CLI-for-RPi-Pico (default assumes standard layout)
if(NOT DEFINED FAT_CLI_PATH)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-FAT-CLI-for-RPi-Pico")
        set(FAT_CLI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/FreeRTOS-FAT-CLI-for-RPi-Pico")

    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../lib/FreeRTOS-FAT-CLI-for-RPi-Pico")
        set(FAT_CLI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../lib/FreeRTOS-FAT-CLI-for-RPi-Pico")

    else()
        message(FATAL_ERROR "Could not locate FreeRTOS-FAT-CLI-for-RPi-Pico.
Set FAT_CLI_PATH manually or check your submodule path.")
    endif()

endif()

message(STATUS "[FAT+CLI] FAT_CLI_PATH = ${FAT_CLI_PATH}")
message(STATUS "[FAT+CLI] FREERTOS_KERNEL_PATH = ${FREERTOS_KERNEL_PATH}")

# Subdirectories
set(LAB_FAT_PATH ${FAT_CLI_PATH}/src/Lab-Project-FreeRTOS-FAT)
set(PORTABLE_PATH ${FAT_CLI_PATH}/src/FreeRTOS+FAT+CLI/portable/RP2040)
set(SRC_PATH ${FAT_CLI_PATH}/src/FreeRTOS+FAT+CLI/src)
set(INCLUDE_PATH ${FAT_CLI_PATH}/src/FreeRTOS+FAT+CLI/include)

add_library(FreeRTOS+FAT+CLI)
target_sources(FreeRTOS+FAT+CLI PRIVATE
    ${LAB_FAT_PATH}/ff_crc.c
    ${LAB_FAT_PATH}/ff_dir.c
    ${LAB_FAT_PATH}/ff_error.c
    ${LAB_FAT_PATH}/ff_fat.c
    ${LAB_FAT_PATH}/ff_file.c
    ${LAB_FAT_PATH}/ff_format.c
    ${LAB_FAT_PATH}/ff_ioman.c
    ${LAB_FAT_PATH}/ff_locking.c
    ${LAB_FAT_PATH}/ff_memory.c
    ${LAB_FAT_PATH}/ff_stdio.c
    ${LAB_FAT_PATH}/ff_string.c
    ${LAB_FAT_PATH}/ff_sys.c
    ${LAB_FAT_PATH}/ff_time.c

    ${PORTABLE_PATH}/dma_interrupts.c
    ${PORTABLE_PATH}/ff_sddisk.c
    ${PORTABLE_PATH}/sd_card.c
    ${PORTABLE_PATH}/SPI/sd_card_spi.c
    ${PORTABLE_PATH}/SPI/sd_spi.c
    ${PORTABLE_PATH}/SPI/my_spi.c
    ${PORTABLE_PATH}/SDIO/sd_card_sdio.c
    ${PORTABLE_PATH}/SDIO/rp2040_sdio.c

    ${SRC_PATH}/crash.c # I may replace this
    ${SRC_PATH}/crc.c
    ${SRC_PATH}/ff_utils.c
    ${SRC_PATH}/file_stream.c
    ${SRC_PATH}/freertos_callbacks.c
    ${SRC_PATH}/FreeRTOS_strerror.c
    ${SRC_PATH}/FreeRTOS_time.c
    ${SRC_PATH}/my_debug.c # I may replace this
    ${SRC_PATH}/sd_timeouts.c
    ${SRC_PATH}/util.c
)

#global include for config files

if (NOT DEFINED FREERTOS_CONFIG_FILE_DIRECTORY)
    message(FATAL_ERROR "[FAT+CLI] FREERTOS_CONFIG_FILE_DIRECTORY is not defined. Pass -DFREERTOS_CONFIG_FILE_DIRECTORY=...")
endif()

message(STATUS "[FAT+CLI] Using FreeRTOSConfig from: ${FREERTOS_CONFIG_FILE_DIRECTORY}")

target_include_directories(FreeRTOS+FAT+CLI PUBLIC
    ${FREERTOS_CONFIG_FILE_DIRECTORY}
    ${INCLUDE_PATH}
    ${LAB_FAT_PATH}/include
    ${PORTABLE_PATH}
    ${FREERTOS_KERNEL_PATH}/include
)

# Conditionally add correct FreeRTOS portmacro include
if(PICO_PLATFORM STREQUAL "rp2040")
    target_include_directories(FreeRTOS+FAT+CLI PUBLIC
        ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include
    )
elseif(PICO_PLATFORM STREQUAL "rp2350-arm-s")
    target_include_directories(FreeRTOS+FAT+CLI PUBLIC
        ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure
    )
endif()

target_link_libraries(FreeRTOS+FAT+CLI PUBLIC
    cmsis_core
    FreeRTOS-Kernel-Heap4
    hardware_adc
    hardware_clocks
    hardware_dma
    hardware_pio
    hardware_spi
    hardware_timer
    pico_aon_timer
    pico_multicore
    pico_stdlib
    pico_sync
)

add_compile_options(-Wno-psabi)
