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

#include <pico/cyw43_arch.h>

    /**
     * @brief Static class providing Wi-Fi and network control on the Pico W.
     */
    class Network
    {
    public:

        /**
         * @brief Start Wi-Fi with resilience, retrying connection if it fails.
         * Uses a static method to handle retries and connection status.
         *
         * @return true if Wi-Fi started successfully, false otherwise.
         */
        static bool startWifiWithResilience();

        /**
         * @brief Attempt to connect to Wi-Fi with retries.
         * This method will try to connect to the Wi-Fi network defined by WIFI_SSID and WIFI_PASSWORD.
         * It will retry up to a maximum number of attempts defined by WIFI_MAX_RETRIES.
         *
         * @param attempts Number of connection attempts to make.
         * @return true if connected, false otherwise.
         */
        static bool checkAndReconnect();

        /**
         * @brief Try to connect to Wi-Fi network.
         * This method will attempt to connect to the Wi-Fi network defined by WIFI_SSID and WIFI_PASSWORD.
         * It will retry up to a maximum number of attempts defined by WIFI_MAX_RETRIES.
         *
         * @param attempts Number of connection attempts to make.
         * @return true if connected, false otherwise.
         */
        static bool tryConnect(int attempts);

        /**
         * @brief Restart the Wi-Fi interface.
         * This method will deinitialize and reinitialize the Wi-Fi stack.
         *
         * @return true if Wi-Fi restarted successfully, false otherwise.
         */
        static bool restart_wifi();

        /**
         * @brief Deinitialize the CYW43 Wi-Fi stack.
         */
        static void wifi_deinit();

        /**
         * @brief Get the current link status from the Wi-Fi interface.
         *
         * @return int CYW43_LINK_UP, CYW43_LINK_DOWN, etc.
         */
        static int getLinkStatus(int lastStatus);

        /**
         * @brief Check whether the device is connected to Wi-Fi.
         *
         * @return true if connected
         */
        static bool isConnected();

        /**
         * @brief Get the IP address.
         *
         * @return true if connected and has IP
         */
        static char *getIpAddress()
        {
            return ip4addr_ntoa(netif_ip4_addr(netif_list));
        }

    private:
        static bool wifiConnected;
    };

#endif /* NETWORK_H */
