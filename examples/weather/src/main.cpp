#include "pico/stdlib.h"

#include "App.h"

App app(80);  

int main() {
    stdio_init_all();
    std::cout << "System Booting..." << std::endl;

    vTaskStartScheduler();

    std::cerr << "ERROR: Scheduler did not start!" << std::endl;
    return 0;
}
