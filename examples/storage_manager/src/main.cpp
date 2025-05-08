#include "pico/stdlib.h"

#include "App.h"

int main() {
    stdio_init_all();

    static App app(80);

    std::cout << "System Booting..." << std::endl;

    vTaskStartScheduler();

    std::cerr << "ERROR: Scheduler did not start!" << std::endl;
    return 0;
}
