/**
 * @file Network.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <pico/cyw43_arch.h>

#include <lwip/ip4_addr.h>

//#include "http_server.hpp"
#include "lwip/tcpip.h"


/**
 * FreeRTOS includes
*/
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "Network.h"

/**
 * 
 * Make the Wi-Fi connection in station mode
 * Requires WIFI_SSID and WIFI_PASSWORD to be set in the environment
 * 
*/

#include "pico/version.h"

bool Network::wifiConnected = false;

void Network::start_wifi(){
    if (cyw43_arch_init()) {
        printf("Failed to initialise Wi-Fi\n");
        return;
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); // large stack increase on first put?

    printf("\n\nConnecting to WiFi SSID: %s \n", WIFI_SSID);

    cyw43_arch_enable_sta_mode();
    uint32_t pm;
    cyw43_wifi_get_pm(&cyw43_state, &pm);
    cyw43_wifi_pm(&cyw43_state, CYW43_DEFAULT_PM & ~0xf); // disable power management
    printf("Connecting to WiFi...\n");

    // connecting asynchronously - need to check link status if up
    if (cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_MIXED_PSK) == 0) {
        printf("\nReady, preparing to connect to network\n");
    }

    int status = 0;
    do {
        status = getLinkStatus();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    } while (status != CYW43_LINK_UP);
    wifiConnected = true;
    printf("Connected to Wi-Fi network at %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
    cyw43_wifi_pm(&cyw43_state, pm); // reset power management
}

bool Network::isConnected() {
    return wifiConnected;
}

void Network::wifi_deinit(){
    cyw43_arch_deinit();
}

int Network::getLinkStatus(){

    int status = cyw43_tcpip_link_status( &cyw43_state,  CYW43_ITF_STA);  
    switch(status){
        case CYW43_LINK_UP:
            printf("Link is up\n");
            break;
        case CYW43_LINK_NOIP:
            printf("Link is up but no IP address\n");
            break;
        case CYW43_LINK_JOIN:
            printf("Link is joining network\n");
            break;
        case CYW43_LINK_DOWN:
            printf("Link is down\n");
            break;
    }
    return status;
}
