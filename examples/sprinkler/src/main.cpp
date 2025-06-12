#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "App.h"


int main() {

    stdio_init_all();

    static App app(80);   

    std::cout << "[main] System Booting, starting scheduler" << std::endl;   
    vTaskStartScheduler();

    std::cerr << "[main] ERROR: Scheduler did not start!" << std::endl;
    return 0;
}