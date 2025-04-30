/**
 * @file FrameworkManager.cpp
 * @author Ian Archbell
 * @brief Framework manager for initializing tasks and triggering App after network ready
 *
 * This module implements the FrameworkManager class, which orchestrates the
 * initialization of core system services, particularly networking. It launches
 * a FreeRTOS task to handle network setup and waits for the network to be
 * up and running. Once the network is ready, it notifies the application
 * task to proceed with its operations.
 *
 * @version 0.2
 * @date 2025-03-26
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */
#include <pico/async_context.h>
#include <pico/async_context_freertos.h>
#if PICO_RP2350
#include "RP2350.h"
#else
#include "RP2040.h"
#endif
#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"       // For trace logging
TRACE_INIT(FrameworkManager); // Initialize tracing for this module
#include "framework/FrameworkManager.h"

#include <iostream>
#include "framework/FrameworkApp.h"
#include "framework/AppContext.h"
#include "framework_config.h"
#include "framework/AppContext.h"
#include "time/TimeManager.h"
#include "events/EventManager.h"
#include "events/Event.h"
#include "events/Notification.h"
#include "utility/utility.h"
#ifdef PICO_HTTP_ENABLE_JWT
#include "http/JwtAuthenticator.h"
#endif // PICO_HTTP_ENABLE_JWT

/// @copydoc FrameworkManager::xNetworkTaskBuffer
StaticTask_t FrameworkManager::xNetworkTaskBuffer;

/// @copydoc FrameworkManager::xNetworkStack
StackType_t FrameworkManager::xNetworkStack[NETWORK_STACK_SIZE];

/// @copydoc FrameworkManager::FrameworkManager
FrameworkManager::FrameworkManager(FrameworkApp *app, Router &router)
    : FrameworkController("FrameworkManager", router, 1024, 2),
      app(app),
      networkTaskHandle(nullptr)
{
    AppContext::getInstance().initFrameworkServices();
}

/// @copydoc FrameworkManager::onStart()
void FrameworkManager::onStart()
{

    setupTraceFromConfig();
    std::cout << "[Framework Manager] Initializing framework..." << std::endl;

    TimeManager *timeMgr = AppContext::get<TimeManager>();
    configASSERT(timeMgr); // Will hard fault early if registration failed
    timeMgr->start();      // If AON timer is running it will post a TimerValid event

    printf("[Framework Manager] Starting WiFi...\n");
    if (!network.startWifiWithResilience())
    {
#if WIFI_REBOOT_ON_FAILURE
        printf("[Framework Manager] WiFi failed — rebooting...\n");
        NVIC_SystemReset();
#else
        printf("[Framework Manager] WiFi failed after retries. Continuing without network.\n");
        return;
#endif
    }
    printf("[Framework Manager] Framework services initialized.\n");

    timeMgr->onNetworkReady(); // tell timemanager so it can do what it needs to do

    printf("[Framework Manager] Network up. Notifying app task...\n");

    Event event;
    event.notification = SystemNotification::NetworkReady;
    AppContext::get<EventManager>()->postEvent(event);

    // Timemenager will handle the time sync and timezone detection
    AppContext::get<EventManager>()->subscribe(eventMask(SystemNotification::HttpServerStarted), this);

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Keep the task alive
        // we are going to delegate events
        // look after the wifi
        // etc here
    }

    // This point should never be reached, but if it is, clean up
    printf("[Framework Manager] Network task exiting unexpectedly.\n");
    vTaskDelete(nullptr);
}

/// @copydoc FrameworkManager::app_task
void FrameworkManager::app_task(void *params)
{
    // Not implemented (placeholder)
    vTaskDelete(nullptr);
}

void FrameworkManager::onEvent(const Event &event)
{
    if (event.notification.kind == NotificationKind::System &&
        event.notification.system == SystemNotification::HttpServerStarted)
    {
        AppContext::get<TimeManager>()->onHttpServerStarted();
    }
}

/**
 * @brief Polling function for the FrameworkManager.
 * This function checks the Wi-Fi connection status at regular intervals
 * and attempts to reconnect if the connection is lost.
 */
void FrameworkManager::poll()
{
#if WIFI_MONITOR_INTERVAL_MS > 0
    static uint32_t lastCheck = 0;
    static int networkFailures = 0; // Track consecutive failures
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if (now - lastCheck >= WIFI_MONITOR_INTERVAL_MS)
    {
        lastCheck = now;

        if (!Network::checkAndReconnect())
        {
            printf("[FrameworkManager] Reconnect failed. Restarting Wi-Fi...\n");

            if (!Network::restart_wifi())
            {
                networkFailures++;

                Event event{SystemNotification::NetworkDown};
                AppContext::get<EventManager>()->postEvent(event);

                if (WIFI_REBOOT_ON_FAILURE && networkFailures >= 3)
                {
                    printf("[FrameworkManager] Rebooting after 3 failed recovery attempts.\n");
                    rebootSystem();
                }
            }
            else
            {
                networkFailures = 0; // success resets the counter
                Event event{SystemNotification::NetworkReady};
                AppContext::get<EventManager>()->postEvent(event);
            }
        }
        else
        {
            networkFailures = 0; // normal path
        }
    }
#endif

    // ... other poll logic ...
}

/// @copydoc FrameworkManager::setupTraceFromConfig
void FrameworkManager::setupTraceFromConfig()
{
#if TRACE_USE_SD
    setTraceOutputToFile(AppContext::getFatFsStorage(), TRACE_LOG_PATH);
#else
    setTraceOutputToFile(nullptr, "");
#endif

#if TRACE_SYSTEM
    TRACE_INIT(SYSTEM);
    TRACE("Tracing initialized from framework.");
#endif
}
