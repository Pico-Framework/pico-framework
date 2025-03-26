#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app.h"
#include "FrameworkManager.h"

#define FRAMEWORK_STACK_SIZE 512
StackType_t xFrameworkStack[FRAMEWORK_STACK_SIZE];
StaticTask_t xFrameworkTaskBuffer;

void framework_task(void* params) {
    static App myApp(80);
    static FrameworkManager manager(&myApp);
    manager.start();
    vTaskDelete(nullptr);
}

int main() {
    stdio_init_all();
    std::cout << "System Booting..." << std::endl;

    TaskHandle_t frameworkTask = xTaskCreateStatic(framework_task, "FrameworkTask", FRAMEWORK_STACK_SIZE, nullptr, 1, xFrameworkStack, &xFrameworkTaskBuffer);
    if (!frameworkTask) {
        std::cerr << "ERROR: Failed to create Framework Task!" << std::endl;
        return -1;
    }

    vTaskStartScheduler();

    std::cerr << "ERROR: Scheduler did not start!" << std::endl;
    return 0;
}
