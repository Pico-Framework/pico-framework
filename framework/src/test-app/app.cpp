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

    EventManager::getInstance().subscribe(mask(SystemNotification::GpioChange), this);
    GpioEventManager::getInstance().enableInterrupt(16, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
    GpioEventManager::getInstance().enableInterrupt(17, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);

    EventManager::getInstance().subscribe(mask(UserNotification::Heartbeat), this);

    std::cout << "[App] Waiting for network..." << std::endl;
    waitFor(SystemNotification::NetworkReady);

    std::cout << "[App] Network ready. Starting services..." << std::endl;
    server.start();
}

void App::onEvent(const Event& e)
{
    if (e.notification.kind == NotificationKind::System &&
        e.notification.system == SystemNotification::GpioChange)
    {
        printf("[App] Raw event: data=%p, size=%zu\n", e.data, e.size);
        const GpioEvent* data = static_cast<const GpioEvent*>(e.data);
        if (e.size == sizeof(GpioEvent)) {
            printf("[App] Pin = %u, Edge = 0x%X\n", data->pin, data->edge);
        } else {
            printf("[App] BAD EVENT: size=%zu (expected %zu)\n", e.size, sizeof(GpioEvent));
        }
        if (e.data && e.size >= sizeof(GpioEvent))
        {
            const GpioEvent* gpioEvent = static_cast<const GpioEvent*>(e.data);
            const char* edgeStr =
                (gpioEvent->edge & GPIO_IRQ_EDGE_RISE) ? "rising" :
                (gpioEvent->edge & GPIO_IRQ_EDGE_FALL) ? "falling" : "unknown";

            printf("[App] GPIO changed - pin %u: %s\n", gpioEvent->pin, edgeStr);
        }
        else
        {
            printf("[App] GpioChange event missing or malformed\n");
        }
    }

    if (e.notification.kind == NotificationKind::User &&
        e.notification.user_code == static_cast<uint8_t>(UserNotification::Heartbeat))
    {
        printf("[App] Heartbeat user event received\n");
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
            Event userEvt(static_cast<uint8_t>(UserNotification::Heartbeat));
            EventManager::getInstance().postEvent(userEvt);
            // this yield is not essential, but without yielding you can miss events
            vTaskDelay(pdMS_TO_TICKS(1)); 
        }, "logLoop"); // <-- Unique ID for this timer (enables it to be cancelled)
}
