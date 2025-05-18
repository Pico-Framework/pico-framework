#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app.h"

int main() {

    int64_t start = to_ms_since_boot(get_absolute_time());
    stdio_init_all();

    static App app(80);   
    int64_t end = to_ms_since_boot(get_absolute_time());
    printf("[BootTimer] App constructed in %lld ms\n", end - start);

    std::cout << "System Booting..." << std::endl;
    
    std::cout << "[main] Starting Scheduler..." << std::endl;
    vTaskStartScheduler();

    std::cerr << "ERROR: Scheduler did not start!" << std::endl;
    return 0;
}