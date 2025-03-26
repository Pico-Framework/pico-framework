#include "system_manager.h"
#include <iostream>
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#define SYSTEM_STACK_SIZE 512

void system_task(void* params) {
    SystemManager framework(80);
    framework.start();
    vTaskDelete(nullptr);
}

StackType_t xSystemStack[ SYSTEM_STACK_SIZE ];
StaticTask_t xSystemTaskBuffer;

int main() {

    stdio_init_all();

    std::cout << "System Booting..." << std::endl;

    // Create a task to initialize SystemManager
    TaskHandle_t systemTask = xTaskCreateStatic(system_task, "SystemTask", SYSTEM_STACK_SIZE, nullptr, TaskPrio_Low, xSystemStack, &xSystemTaskBuffer);
    if(systemTask == nullptr) {
        std::cerr << "ERROR: Failed to create System Task!" << std::endl;
        return -1;
    }
    else{
        std::cout << "System Task created successfully!" << std::endl;
    }

    // Start FreeRTOS Scheduler
    vTaskStartScheduler();

    std::cerr << "ERROR: Scheduler did not start!" << std::endl;
    //while (1) {}

    return 0;
}
