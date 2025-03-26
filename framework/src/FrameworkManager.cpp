/**
 * @file FrameworkManager.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "FrameworkManager.h"
#include "FrameworkApp.h" 
#include <iostream>
#include "utility.h"  // If needed
#include "Task.h"     // If needed

StaticTask_t FrameworkManager::xApplicationTaskBuffer;   
StaticTask_t FrameworkManager::xNetworkTaskBuffer;
StackType_t FrameworkManager::xNetworkStack[NETWORK_STACK_SIZE];
StackType_t FrameworkManager::xApplicationStack[APPLICATION_STACK_SIZE];

FrameworkManager::FrameworkManager(FrameworkApp* app)
    : app(app), networkTaskHandle(nullptr), applicationTaskHandle(nullptr) {}

void FrameworkManager::start() {
    std::cout << "Initializing framework..." << std::endl;
    
    networkTaskHandle = xTaskCreateStatic(network_task, "NetworkTask", NETWORK_STACK_SIZE, this, 3, xNetworkStack, &xNetworkTaskBuffer);
    applicationTaskHandle = xTaskCreateStatic(app_task, "AppTask", APPLICATION_STACK_SIZE, this, 2, xApplicationStack, &xApplicationTaskBuffer);
}

void FrameworkManager::network_task(void* params) {
    auto* manager = static_cast<FrameworkManager*>(params);

    std::cout << "Starting WiFi..." << std::endl;
    manager->network.start_wifi(); // replace with actual network logic

    while (!manager->network.isConnected()) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelay(pdMS_TO_TICKS(1000)); // Simulate Wi-Fi connect

    std::cout << "Network up. Notifying app task..." << std::endl;
    if (manager->applicationTaskHandle != nullptr) {
        xTaskNotifyGive(manager->applicationTaskHandle);
    }

    vTaskDelete(nullptr);
}

void FrameworkManager::app_task(void* params) {
    auto* manager = static_cast<FrameworkManager*>(params);

    std::cout << "Waiting for network to be ready..." << std::endl;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    std::cout << "Network ready. Starting app..." << std::endl;
    manager->app->initRoutes();
    manager->app->run();

    vTaskDelete(nullptr);
}
