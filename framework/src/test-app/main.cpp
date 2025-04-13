#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "app.h"

App myApp(80);  

int main() {
    stdio_init_all();
    
    std::cout << "System Booting..." << std::endl;
    myApp.start();  // Creates the app task and starts framework manager
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