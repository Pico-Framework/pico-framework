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

 #include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
 #include "DebugTrace.h" // For trace logging
 TRACE_INIT(FrameworkManager); // Initialize tracing for this module
 #include "FrameworkManager.h"
 #include "FrameworkApp.h"
 #include "AppContext.h"
 #include <iostream>
 #include "framework_config.h"
 
 /// @copydoc FrameworkManager::xNetworkTaskBuffer
 StaticTask_t FrameworkManager::xNetworkTaskBuffer;
 
 /// @copydoc FrameworkManager::xNetworkStack
 StackType_t FrameworkManager::xNetworkStack[NETWORK_STACK_SIZE];
 
 /// @copydoc FrameworkManager::FrameworkManager
 FrameworkManager::FrameworkManager(FrameworkApp* app)
     : app(app), networkTaskHandle(nullptr) {}
 
 /// @copydoc FrameworkManager::start
 void FrameworkManager::start() {
     setupTraceFromConfig();
     std::cout << "Initializing framework..." << std::endl;

 
     // Launch only the network task
     networkTaskHandle = xTaskCreateStatic(
         network_task,
         "NetworkTask",
         NETWORK_STACK_SIZE,
         this,
         3,
         xNetworkStack,
         &xNetworkTaskBuffer
     );
 }
 
 /// @copydoc FrameworkManager::network_task
 void FrameworkManager::network_task(void* params) {
     auto* manager = static_cast<FrameworkManager*>(params);
 
     std::cout << "Starting WiFi..." << std::endl;
     manager->network.start_wifi(); // Replace with actual network logic
 
     while (!manager->network.isConnected()) {
         vTaskDelay(pdMS_TO_TICKS(1000));
     }
 
     std::cout << "Network up. Notifying app task..." << std::endl;
 
     if (manager->app != nullptr) {
         manager->app->notify(FrameworkNotification::NetworkReady);
     }
 
     vTaskDelete(nullptr);  // Done
 }
 
 /// @copydoc FrameworkManager::app_task
 void FrameworkManager::app_task(void* params) {
     // Not implemented (placeholder)
     vTaskDelete(nullptr);
 }
/// @copydoc FrameworkManager::setupTraceFromConfig
 void FrameworkManager::setupTraceFromConfig() {
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
 