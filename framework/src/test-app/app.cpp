#include "App.h"
#include <hardware/adc.h>
#include <pico/cyw43_arch.h>
#include "AppContext.h"
#include "StorageManager.h"

#include "GpioController.h"
#include "DashboardController.h"
#include "GpioEvent.h"
#include "EventManager.h"
#include "GpioEventManager.h"
// #include <iostream>

App::App(int port) : FrameworkApp(port, "AppTask", 1024, 1)
{
}

void App::initRoutes()
{
    router.addRoute("GET", "/hello", [](HttpRequest &req, HttpResponse &res, const auto &)
                    { res.send("Welcome to PicoFramework!"); });
}

void App::onStart()
{
    FrameworkApp::onStart();

    static GpioController gpioController(routerInstance);
    gpioController.start();
    static DashboardController dashboardController(routerInstance);
    dashboardController.start();

    EventManager::getInstance().subscribe(mask(EventType::GpioChange), this);
    GpioEventManager::getInstance().enableInterrupt(16, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);

    std::cout << "[App] Waiting for network..." << std::endl;
    waitFor(FrameworkNotification::NetworkReady);

    std::cout << "[App] Network ready. Starting services..." << std::endl;
    server.start();
}

void App::onEvent(const Event &e)
{
    if (e.type == EventType::GpioChange)
    {
        const GpioEvent *data = static_cast<const GpioEvent *>(e.data);
        printf("[App] GPIO changed - pin %d: %s\n",
               data->pin,
               (data->edge & GPIO_IRQ_EDGE_RISE) ? "rising" : (data->edge & GPIO_IRQ_EDGE_FALL) ? "falling" : "unknown");
    }
}

void App::poll()
{
    vTaskDelay(pdMS_TO_TICKS(100)); // Yield to other tasks
    // This function is called continuously (non-blocking) from the App task.
    // Use RUN_EVERY to do periodic background work:

    runEvery(15000, [&]()
        { 
            printf("[App] Running main polling loop...\n"); 
            // this yield is not essential, but without yielding you can miss events
            vTaskDelay(pdMS_TO_TICKS(1)); 
        }, "logLoop"); // <-- Unique ID for this timer (enables it to be cancelled)
}
