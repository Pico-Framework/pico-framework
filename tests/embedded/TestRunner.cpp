#include "CppUTest/CommandLineTestRunner.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "network/Network.h"
#include "lwip/tcpip.h"
#include "pico/stdlib.h"    

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

extern "C" void runStorageManagerTests();

void testTask(void*) {
    // printf("Starting test task...\n");
    // Network::startWifiWithResilience(); // Initialize Wi-Fi
    // while(!Network::isConnected()) {
    //     vTaskDelay(pdMS_TO_TICKS(100)); // Wait for Wi-Fi connection
    // }
    // printf("Wi-Fi connected. Running tests...\n");
    // manually invoke the tests after scheduler is running
    sleep_ms(500);
    runStorageManagerTests();

    //bool result = CommandLineTestRunner::RunAllTests(0, (char**)nullptr);
    //printf("Test task completed with result %d\n", result);
    vTaskSuspend(nullptr); // Freeze here after tests
}

int main() {
    stdio_init_all();
    printf("Starting FreeRTOS test runner...\n");

    xTaskCreate(testTask, "TestTask", 1024, nullptr, tskIDLE_PRIORITY + 1, nullptr);
    vTaskStartScheduler();

    while (1) {} // Should never reach
}
