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


/// @copydoc FrameworkManager::FrameworkManager
FrameworkManager::FrameworkManager(FrameworkApp *app, Router &router)
    : FrameworkController("FrameworkManager", router, 1024, 2),
      app(app),
      networkTaskHandle(nullptr)
{
}

/// @copydoc FrameworkManager::onStart()
void FrameworkManager::onStart()
{
    setupTraceFromConfig();
    std::cout << "[Framework Manager] Initializing framework..." << std::endl;

    // It is important to ensure that the AppContext is initialized
    // before we start using it in the Framework.
    // For example EventManager and other application service must be available for the user in onStart();
    AppContext::getInstance().initFrameworkServices();

    TimeManager *timeMgr = AppContext::get<TimeManager>();
    configASSERT(timeMgr); // Will hard fault early if registration failed
    timeMgr->start();      // If AON timer is running it will post a TimerValid event

    // TimeManager will handle the time sync and timezone detection
    AppContext::get<EventManager>()->subscribe(eventMask(SystemNotification::HttpServerStarted), this);

    if (!Network::initialize())
    {
        printf("[Framework Manager] Failed to initialize network stack.\n");
    }

    warmUp(); // Warm up the JSON parser and other components

    // needs to be started after scheduler running to ensure full use of FreeRTOS
    app->start(); // Starts the app task 

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
}

/// @copydoc FrameworkManager::warmUp
void FrameworkManager::warmUp()
{
    // Warm up JSON parser
    {
        nlohmann::json j = nlohmann::json::parse("{\"warmup\":true}", nullptr, false);
        (void)j.dump(); // Force stringify
    }

    // Warm up HttpRequest
    {
        HttpRequest dummy;
        dummy.setMethod("GET");
        dummy.setPath("/warmup");
        dummy.setHeader("X-Warmup", "true");
        (void)dummy.getHeader("X-Warmup");
    }

    // Force common string ops
    std::string("warmup");

    // Force a task yield
    vTaskDelay(pdMS_TO_TICKS(1));
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
        printf("[FrameworkManager] HttpServer started, notifying TimeManager...\n");
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
        printf("[FrameworkManager] Polling for Wi-Fi status...\n");

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
