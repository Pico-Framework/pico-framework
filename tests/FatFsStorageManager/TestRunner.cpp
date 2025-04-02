#include "CppUTest/CommandLineTestRunner.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

extern "C" void stdio_init_all(); // Optional for UART/USB output

void testTask(void*) {
    printf("Starting test task...\n");
    bool result = CommandLineTestRunner::RunAllTests(0, (char**)nullptr);
    printf("Test task completed with result %d\n", result);
    vTaskSuspend(nullptr); // Freeze here after tests
}

int main() {
    stdio_init_all();
    xTaskCreate(testTask, "TestTask", 4096, nullptr, tskIDLE_PRIORITY + 1, nullptr);
    vTaskStartScheduler();

    while (1) {} // Should never reach
}
