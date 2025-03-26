#include "system_manager.h"
#include <iostream>
//#include "PicoTime.h"
#include "utility.h"
#include "Task.h"
#include "FreeRTOS.h"
#include "app.h"

SystemManager::SystemManager(int port) : app(port), networkTaskHandle(nullptr), applicationTaskHandle(nullptr) {}

StaticTask_t SystemManager::xApplicationTaskBuffer;   
StaticTask_t SystemManager::xNetworkTaskBuffer;
StackType_t SystemManager::xNetworkStack[ NETWORK_STACK_SIZE ];
StackType_t SystemManager::xApplicationStack[ APPLICATION_STACK_SIZE ];

void SystemManager::start() {
    std::cout << "Initializing system..." << std::endl;
    
    networkTaskHandle = xTaskCreateStatic(network_task, "NetworkTask", NETWORK_STACK_SIZE, this, TaskPrio_High, xNetworkStack, &xNetworkTaskBuffer);
    applicationTaskHandle = xTaskCreateStatic(app_task, "AppTask", APPLICATION_STACK_SIZE, this, TaskPrio_Mid, xApplicationStack, &xApplicationTaskBuffer);
}

void SystemManager::network_task(void* params) {
    SystemManager* manager = static_cast<SystemManager*>(params);
    std::cout << "Starting WiFi..." << std::endl;
    manager->network.start_wifi();

    while (!manager->network.isConnected()) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    std::cout << "Notifying App task that Wi-Fi is up" << std::endl;
    if (manager->applicationTaskHandle != nullptr) {
        xTaskNotifyGive(manager->applicationTaskHandle);
    } else {
        std::cerr << "Error: ApplicationTaskHandle is NULL!" << std::endl;
    }

    vTaskDelay(pdMS_TO_TICKS(2000)); // Ensure network stability
    manager->ntpClient.requestTime();

    std::cout << "Network task completed successfully." << std::endl;
    vTaskDelete(nullptr);
}

void SystemManager::app_task(void* params) {
    SystemManager* manager = static_cast<SystemManager*>(params);
    
    std::cout << "Waiting for network to be ready in App task..." << std::endl;
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    std::cout << "Network is ready! Starting application..." << std::endl;
    
    manager->app.run();
    
    std::cout << "App task finished execution!" << std::endl;
    vTaskDelete(nullptr);
}
