/**
 * @file hw_config.c
 * @author Carl John Kugler III
 * @brief Hardware configuration for SDIO interface and card setup.
 *
 * This config is shipped with the PicoFramework and is used to configure the
 * SD card interface using the SDIO protocol. It defines the GPIO pins used for
 * command (CMD) and data lines, as well as the card detection GPIO.
 * 
 * You will need to edit this file to match your specific hardware setup.
 * It used by Carl John Kugler III in his adaptaion of Fat Fs for the Pico SDIO
 * implementation. See his original work at:
 * 
 * https://github.com/carlk3/FreeRTOS-FAT-CLI-for-RPi-Pico
 * 
 * FatFsStorageManager is uses this library as a submodule for SDIO. It is possible to use PSI 
 * but not recommended for large filesizes. SDIO on the pico is much faster (4 bits vs 1 bit).
 * 
 * This file configures SDIO GPIO mapping and provides access to the `sd_card_t`
 * instance used by the storage system. The GPIO pin mapping must match the PIO
 * implementation used by your SD card interface.
 *
 * This is adapted for use with the Pico SDIO module, and may require customization
 * for other hardware.
 *
 * @version 1.0
 * @date 2021-xx-xx (original), modified 2025-03-31 by Ian Archbell
 * @license Apache License 2.0
 * @copyright Copyright (c) 2021 Carl John Kugler III
 */

 #include "hw_config.h"

 /**
  * 
  * PicoFramework Storage Configuration
  * 
  * You can use either littlefs flash storage or SD card storage with the PicoFramework.
  * If you choose to use SD card storage, you will need to configure the SDIO or SPI interface.
  * Carl John Kugler III has adapted the FatFs library for use with the Raspberry Pi Pico,
  * and we use it as a submodule. the "hw_config.c" file is used to configure the SD card interface.
  * 
  * Note that his library supports multipe cards. The PicoFramework does not. You can only use one SD card.
  * 
  * This configuration as shipped is setup to use SDIO on the Raspberry Pi Pico
  * with a Pico Expansion Plus S1 board (available on Amazon) or similar board that has the SDIO pins
  * connected to the SD card slot in sequental fashion.
  * 
  * Only boards that have consecutive GPIO pins connected to the SDIO interface
  * will work with this library in SDIO mode. This restriction does not a apply to SPI mode.
  * 
  * We are considering putting out a custom board that has the SDIO pins connected to a microSD slot
  * in the future, but for now this is a good starting point for using SDIO on the Pico. Our board will also
  * include battery-backed RTC and other features that make it suitable for IoT applications.
  * Let us know at PicoFramework.com.
  * 
  * The SDIO interface uses the following GPIO pins:
  * 
  * CLK: GPIO 10
  * CMD: GPIO 11
  * DAT0: GPIO 12
  * DAT1: GPIO 13
  * DAT2: GPIO 14
  * DAT3: GPIO 15
  * 
  * You can find out how to configure an SPI SD card interface at 
  * Carl's Github page:
  * 
  * https://github.com/carlk3/FreeRTOS-FAT-CLI-for-RPi-Pico
  * 
  */

 // SDIO Interface configuration
 static sd_sdio_if_t sdio_if = {
     .CMD_gpio = 11,
     .D0_gpio = 12
 };
 
 // SD card configuration
 static sd_card_t sd_card = {
     .device_name = "sd0",
     .mount_point = "/sd0",
     .type = SD_IF_SDIO,
     .sdio_if_p = &sdio_if,
     .use_card_detect = false,
     //.card_detect_gpio = 9,
     //.card_detected_true = 0
 };
 
 /**
  * @brief Returns the number of available sd_card_t instances.
  * @return Number of cards (always 1 in this config).
  */
 size_t sd_get_num(void) {
     return 1;
 }
 
 /**
  * @brief Get sd_card_t by index (currently only index 0 is valid).
  * @param num Index of card (0-based).
  * @return Pointer to card config or NULL.
  */
 sd_card_t *sd_get_by_num(size_t num) {
     if (num == 0) return &sd_card;
     return NULL;
 }
 