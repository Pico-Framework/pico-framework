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
#include "pico/async_context.h"
#include "pico/async_context_freertos.h"
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
#ifdef PICO_HTTP_ENABLE_JWT
#include "http/JwtAuthenticator.h"
#endif // PICO_HTTP_ENABLE_JWT

/// @copydoc FrameworkManager::xNetworkTaskBuffer
StaticTask_t FrameworkManager::xNetworkTaskBuffer;

/// @copydoc FrameworkManager::xNetworkStack
StackType_t FrameworkManager::xNetworkStack[NETWORK_STACK_SIZE];

/// @copydoc FrameworkManager::FrameworkManager
FrameworkManager::FrameworkManager(FrameworkApp* app, Router& router)
    : FrameworkController("FrameworkManager", router, 1024, 2),
      app(app),
      networkTaskHandle(nullptr){
        AppContext::getInstance().initFrameworkServices();
      }

/// @copydoc FrameworkManager::onStart()
void FrameworkManager::onStart()
{    
    
    setupTraceFromConfig();
    std::cout << "[Framework Manager] Initializing framework..." << std::endl;

    TimeManager *timeMgr = AppContext::get<TimeManager>();
    configASSERT(timeMgr);  // Will hard fault early if registration failed
    timeMgr->start(); // If AON timer is running it will post a TimerValid event

    printf("[Framework Manager] Starting WiFi...\n");
    network.start_wifi();

    while (!network.isConnected())
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    printf("[Framework Manager] Framework services initialized.\n");

    timeMgr->onNetworkReady(); // Notify TimeManager that network is ready

    printf("[Framework Manager] Network up. Notifying app task...\n");

    Event event;
    event.notification = SystemNotification::NetworkReady;
    AppContext::get<EventManager>()->postEvent(event);  
    
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

void FrameworkManager::onEvent(const Event& event) {
    if (event.notification.kind == NotificationKind::System &&
        event.notification.system == SystemNotification::HttpServerStarted)
    {
        AppContext::get<TimeManager>()->onHttpServerStarted();
    }
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
