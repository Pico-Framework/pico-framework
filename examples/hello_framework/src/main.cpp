#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "App.h"  

int main() {
    stdio_init_all();
    std::cout << "[main] System Booting..." << std::endl;

    static App myApp(80);  // Create an instance of the app on port 80

    vTaskStartScheduler();

    std::cerr << "ERROR: Scheduler did not start!" << std::endl;
    return 0;
}
