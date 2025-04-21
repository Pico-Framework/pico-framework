#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app.h"

//App myApp(80);  

int main() {

    int64_t start = to_ms_since_boot(get_absolute_time());
    stdio_init_all();

    static App app(80);  // <-- construct in main(), safe
    //App* app = new App(80); // <-- or use dynamic allocation, but ensure to delete later
    int64_t end = to_ms_since_boot(get_absolute_time());
    printf("[BootTimer] App constructed in %lld ms\n", end - start);

    std::cout << "System Booting..." << std::endl;
    app.start();  // Creates the app task and starts framework manager
    // note that the scheduler hasn't started yet, start method must not use any FreeRTOS API 
    // calls that would require the scheduler to be running. (e.g. vTaskDelay(...))
    // app inherits from FrameworkApp, whose construction will create the router, server, and framework manager.
    // This ensures that the framework is initialized before the users controller(s).
    // The FrameworkManager will start the network task, which will notify the app when the network is ready.
    
    std::cout << "Starting Scheduler..." << std::endl;
    vTaskStartScheduler();

    std::cerr << "ERROR: Scheduler did not start!" << std::endl;
    return 0;
}