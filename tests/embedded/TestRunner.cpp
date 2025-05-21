#include "CppUTest/CommandLineTestRunner.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "network/Network.h"
#include "lwip/tcpip.h"

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

extern "C" void stdio_init_all(); // Optional for UART/USB output

void testTask(void*) {
    printf("Starting test task...\n");
    Network::startWifiWithResilience(); // Initialize Wi-Fi
    while(!Network::isConnected()) {
        vTaskDelay(pdMS_TO_TICKS(100)); // Wait for Wi-Fi connection
    }
    printf("Wi-Fi connected. Running tests...\n");

    bool result = CommandLineTestRunner::RunAllTests(0, (char**)nullptr);
    printf("Test task completed with result %d\n", result);
    vTaskSuspend(nullptr); // Freeze here after tests
}

int main() {
    stdio_init_all();

    printf("Wi-Fi connected. Starting tests...\n");

    xTaskCreate(testTask, "TestTask", 1024, nullptr, tskIDLE_PRIORITY + 1, nullptr);
    vTaskStartScheduler();

    while (1) {} // Should never reach
}
