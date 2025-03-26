#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app.h"

App myApp(80);  // Safe again!

int main() {
    stdio_init_all();
    std::cout << "System Booting..." << std::endl;

    myApp.start();  // Creates the app task and starts framework manager

    vTaskStartScheduler();

    std::cerr << "ERROR: Scheduler did not start!" << std::endl;
    return 0;
}
