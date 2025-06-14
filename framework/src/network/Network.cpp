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

#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"
TRACE_INIT(Network)

#include "network/Network.h"
#include <pico/cyw43_arch.h>
#include <FreeRTOS.h>
#include <task.h>

/**
 *
 * Make the Wi-Fi connection in station mode
 * Requires WIFI_SSID and WIFI_PASSWORD to be set in the environment
 *
 */

#include "pico/version.h"

/// @brief  @copydoc Network::wifiConnected
bool Network::wifiConnected = false;

/// @brief @copydoc Network::wifiInitialized
bool Network::wifiInitialized = false;

bool Network::initialize()
{
    if (cyw43_arch_init())
    {
        printf("[Network] Failed to initialise Wi-Fi\n");
        return false;
    }

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); // LED off during startup
    cyw43_arch_enable_sta_mode();
    wifiInitialized = true;
    printf("[Network] Wi-Fi initialized successfully.\n");
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); // LED on after startup
    return true;
}

/// @copydoc Network::start_wifi
//  @note This function will block until the Wi-Fi connection is established.
//  @note Make sure to include the necessary headers and define WIFI_SSID and WIFI_PASSWORD
//  in your build environment. For security, it is best defined in your evironment and
//  not hardcoded in the source code.

bool Network::startWifiWithResilience()
{
    if (!wifiInitialized)
    {
        if (!initialize())
        {
            return false;
        }
        wifiInitialized = true;
    }

    bool success = tryConnect(WIFI_MAX_RETRIES);

    if (!success)
    {
        cyw43_arch_deinit(); // Only on total failure
    }

    return success;
}

bool Network::checkAndReconnect()
{
    if (isConnected() && getLinkStatus(CYW43_LINK_UP) == CYW43_LINK_UP)
    {
        return true; // Already connected
    }

    printf("[Network] Connection lost. Attempting reconnect...\n");

    cyw43_arch_enable_sta_mode(); // do not re-init or deinit
    return tryConnect(1);         // One-shot reconnect attempt
}

bool Network::tryConnect(int attempts)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); // LED during all connect attempts

    uint32_t pm;
    cyw43_wifi_get_pm(&cyw43_state, &pm);
    cyw43_wifi_pm(&cyw43_state, CYW43_DEFAULT_PM & ~0xf); // disable power mgmt during connect

    for (int i = 0; i < attempts; ++i)
    {
        printf("\n\n[Network] Connecting to WiFi SSID: %s (attempt %d)\n", WIFI_SSID, i + 1);

        if (cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_MIXED_PSK) != 0)
        {
            printf("[Network] Failed to initiate connection.\n");
            vTaskDelay(pdMS_TO_TICKS(WIFI_RETRY_TIMEOUT_MS));
            continue;
        }

        int status = 0;
        int waitMs = 0;
        do
        {
            status = Network::getLinkStatus(status);
            vTaskDelay(pdMS_TO_TICKS(1000));
            waitMs += 1000;
        } while (status != CYW43_LINK_UP && waitMs < WIFI_RETRY_TIMEOUT_MS);

        if (status == CYW43_LINK_UP)
        {
            wifiConnected = true;
            printf("[Network] Connected to Wi-Fi network at %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
            cyw43_wifi_pm(&cyw43_state, pm);
            return true;
        }

        printf("[Network] Attempt %d failed.\n", i + 1);
    }

    wifiConnected = false;
    cyw43_wifi_pm(&cyw43_state, pm);
    return false;
}

/// @copydoc Network::isConnected
//  @note This function checks the connection status of the Wi-Fi interface.
//  It returns true if the device is connected to a Wi-Fi network, and false otherwise.
bool Network::isConnected()
{
    return wifiConnected;
}

bool Network::restart_wifi()
{
    printf("[Network] Forcing Wi-Fi restart...\n");

    cyw43_arch_deinit();
    wifiConnected = false;

    return startWifiWithResilience();
}

/// @copydoc Network::wifi_deinit
void Network::wifi_deinit()
{
    cyw43_arch_deinit();
}

/// @copydoc Network::getLinkStatus
//  @note This function retrieves the current link status of the Wi-Fi interface.
//  It returns an integer representing the link status, which can be one of the following:
//  - CYW43_LINK_UP: The link is up and connected to a network.
//  - CYW43_LINK_NOIP: The link is up but no IP address has been acquired.
//  - CYW43_LINK_JOIN: The device is in the process of joining a network.
//  - CYW43_LINK_DOWN: The link is down and not connected to any network.
//  @note The function prints the current link status to the console for debugging purposes.
int Network::getLinkStatus(int lastStatus)
{

    int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    switch (status)
    {
    case CYW43_LINK_UP:
        printf("\n[Network] Link is up\n");
        break;

    case CYW43_LINK_NOIP:
        if (lastStatus == CYW43_LINK_NOIP)
        {
            printf(".");
        }
        else
        {
            printf("\n[Network] Acquiring IP address ");
        }
        break;

    case CYW43_LINK_JOIN:
        if (lastStatus == CYW43_LINK_JOIN)
        {
            printf(".");
        }
        else
        {
            printf("\n[Network] Joining network ");
        }
        break;

    case CYW43_LINK_DOWN:
        printf("\n[Network] Link is down\n");
        break;
    }
    return status;
}
