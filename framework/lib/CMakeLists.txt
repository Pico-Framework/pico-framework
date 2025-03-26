# had to add these here as add subdirectory compains with relatve paths
add_library(FreeRTOS+FAT+CLI)

message("Current source directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message("Current list directory: ${CMAKE_CURRENT_LIST_DIR}")
message("Current binary directory: ${CMAKE_CURRENT_BINARY_DIR}")
message("Top-level source directory: ${CMAKE_SOURCE_DIR}")
message("Top-level binary directory: ${CMAKE_BINARY_DIR}")

target_sources(FreeRTOS+FAT+CLI PRIVATE
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_crc.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_dir.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_error.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_fat.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_file.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_format.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_ioman.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_locking.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_memory.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_stdio.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_string.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_sys.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/ff_time.c 
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/portable/RP2040/dma_interrupts.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/portable/RP2040/ff_sddisk.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/portable/RP2040/sd_card.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/portable/RP2040/SPI/sd_card_spi.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/portable/RP2040/SPI/sd_spi.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/portable/RP2040/SPI/my_spi.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/portable/RP2040/SDIO/sd_card_sdio.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/portable/RP2040/SDIO/rp2040_sdio.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/src/crash.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/src/crc.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/src/ff_utils.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/src/file_stream.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/src/freertos_callbacks.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/src/FreeRTOS_strerror.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/src/FreeRTOS_time.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/src/my_debug.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/src/sd_timeouts.c
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/src/util.c
)
target_link_libraries(FreeRTOS+FAT+CLI PRIVATE
        cmsis_core
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        hardware_adc
        hardware_clocks
        hardware_dma
        hardware_pio
        #hardware_rtc
        hardware_spi
        hardware_timer
        pico_aon_timer
        pico_multicore
        pico_stdlib
        pico_sync
        
)

target_include_directories(FreeRTOS+FAT+CLI PRIVATE    
        ${CMAKE_SOURCE_DIR}/framework/include/port
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/Lab-Project-FreeRTOS-FAT/include/
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/portable/RP2040/
        FreeRTOS-FAT-CLI-for-RPi-Pico/src/FreeRTOS+FAT+CLI/include 
    
)
