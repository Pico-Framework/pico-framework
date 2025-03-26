/**
 * @file FrameworkManager.cpp
 * @author Ian Archbell
 * @brief Framework manager for initializing tasks and triggering App after network ready
 * @version 0.2
 * @date 2025-03-26
 * 
 */

 #include "FrameworkManager.h"
 #include "FrameworkApp.h" 
 #include <iostream>
 
 StaticTask_t FrameworkManager::xNetworkTaskBuffer;
 StackType_t FrameworkManager::xNetworkStack[NETWORK_STACK_SIZE];
 
 FrameworkManager::FrameworkManager(FrameworkApp* app)
     : app(app), networkTaskHandle(nullptr) {}
 
 void FrameworkManager::start() {
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
 
 void FrameworkManager::network_task(void* params) {
     auto* manager = static_cast<FrameworkManager*>(params);
 
     std::cout << "Starting WiFi..." << std::endl;
     manager->network.start_wifi(); // Replace with actual network logic
 
     while (!manager->network.isConnected()) {
         vTaskDelay(pdMS_TO_TICKS(1000));
     }
 
     std::cout << "Network up. Notifying app task..." << std::endl;
 
     if (manager->app != nullptr) {
         manager->app->notify(1);  // Notify app's FrameworkTask
     }
 
     vTaskDelete(nullptr);  // Done
 }
 