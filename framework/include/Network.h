/**
 * @file Network.h
 * @author Ian Archbell
 * @brief Manages Wi-Fi initialization, connection status, and power management for Pico W.
 *
 * Part of the PicoFramework application framework.
 * Provides a static interface for connecting to Wi-Fi in station mode using the CYW43 driver.
 * Uses FreeRTOS for async control and polling during link setup. Integrates with the lwIP network stack.
 * 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #ifndef NETWORK_H
 #define NETWORK_H
 #pragma once
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Static class providing Wi-Fi and network control on the Pico W.
  */
 class Network {
 public: 
     /**
      * @brief Initialize Wi-Fi in station mode and connect to the SSID specified by WIFI_SSID and WIFI_PASSWORD.
      * Blocks until link is up and IP address is acquired.
      */
     static void start_wifi();
 
     /**
      * @brief Deinitialize the CYW43 Wi-Fi stack.
      */
     static void wifi_deinit();
 
     /**
      * @brief Get the current link status from the Wi-Fi interface.
      * 
      * @return int CYW43_LINK_UP, CYW43_LINK_DOWN, etc.
      */
     static int getLinkStatus();
 
     /**
      * @brief Check whether the device is connected to Wi-Fi.
      * 
      * @return true if connected
      */
     static bool isConnected();
 
 private:
     static bool wifiConnected;
 };
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* NETWORK_H */
 