/**
 * @file App.cpp
 */

#include "App.h"


#include <hardware/adc.h>
#include <pico/cyw43_arch.h>

#include "framework/AppContext.h"
#include "storage/StorageManager.h"
#include "events/GpioEvent.h"
#include "events/EventManager.h"
#include "events/GpioEventManager.h"
#include "http/HttpFileserver.h"
#include "http/JsonResponse.h"
#include "http/Router.h"

#include "GpioController.h"
#include "DashboardController.h"
#include "DashboardView.h"

// #include <iostream>

App::App(int port) : FrameworkApp(port, "AppTask", 1024, 3)
{
    

}

void App::initRoutes()
{
    // This method is called by the base class to initialize HTTP routes.
    // You can add your application's routes here.
    // All FrameWorkController instances will have their initRoutes called
    // when the application starts, so you can add routes in your controllers.
    // See the GpioController and DashboardController classes for examples.
    // Add a simple route for testing
    router.addRoute("GET", "/", [](HttpRequest &req, HttpResponse &res, const auto &)
                    { res.send(DashboardView()); });
    router.addRoute("GET", "/hello", [](HttpRequest &req, HttpResponse &res, const auto &)
                    { res.send("Welcome to PicoFramework!"); });
    router.addRoute("GET", "/zones/{name}", [](HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
        if (auto name = match.getParam("name")) {
            printf("Named zone: %s\n", name->c_str());
            JsonResponse::sendSuccess(res, {{"zone", *name}});
        } else {
            JsonResponse::sendError(res, 400, "MISSING_NAME", "No zone name provided");
        }
    });
}

void App::onStart()
{
    // Call the base class to ensure the framework starts correctly.
    FrameworkApp::onStart();  
    
    pico.onStart(); // Initialize the PicoModel, which handles GPIO and other hardware interactions

    // These are two controllers derived from FrameworkController that add event support to the FrameworkTask base class.
    // They are started here to ensure they are ready to handle events and HTTP requests.
    std::cout << "[App] Initializing application..." << std::endl;

    static GpioController gpioController(router, pico);
    printf("[App] Starting GPIO controller...\n");
    gpioController.start();
    static DashboardController dashboardController(router, pico);
    printf("[App] Starting Dashboard controller...\n");
    dashboardController.start();

    // Here we are setting up event handlers - we get the EventManager and GpioEventManager from the AppContext.
    EventManager* eventManager = AppContext::get<EventManager>();
    GpioEventManager* gpioEventManager = AppContext::get<GpioEventManager>();
    printf("GpioEventManager = %p\n", gpioEventManager);

    // Subscribe to GPIO change events
    // This will call onEvent() in this App class when a GPIO change event occurs.
    // Note: The GpioEventManager will call the gpio_event_handler() function when a GPIO interrupt occurs.
    // The gpio_event_handler() will then post an Event to the EventManager, which posts to the FRameworkController event queue.
    // FrameworkController has a waitAndDispatch function that polls the queue and will call onEvent() in this App class.
    // Events can be SystemNotification or UserNotification types, which are defined in enum class UserNotification (see the App.h file).
    eventManager->subscribe(eventMask(SystemNotification::GpioChange), this);
    gpioEventManager->enableInterrupt(16, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL); 
    gpioEventManager->enableInterrupt(17, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL); 

    eventManager->subscribe(eventMask(UserNotification::Heartbeat), this);

    std::cout << "[App] Waiting for network..." << std::endl;
    // Wait for the network to be ready before starting services
    // This is a blocking call that waits for the NetworkReady notification
    // You could use a non-blocking wait if you want to do other work in the meantime
    // waitfor is provided by the FrameworkTask base class and has a timeout parameter, unused here
    // The underlying imlementation uses FreeRTOS notifications
    //waitFor(SystemNotification::NetworkReady);

    //std::cout << "[App] Network ready. Starting services..." << std::endl;
    // Now that the network is ready, we can start services that depend on it
    // For example, you might want to start an HTTP server or other network services here
    //server.start();
    eventManager->subscribe(
        eventMask(
            SystemNotification::NetworkReady,
            SystemNotification::TimeValid,
            SystemNotification::TimeSync,
            SystemNotification::TimeInvalid
        ),
        this);


}

void App::onEvent(const Event &e)
{
    // This method is called when an event is posted to the App's event queue.
    // It is called by the FrameworkController base class when an event is received.
    // You can handle different types of events here based on the notification kind.
    // For example, you can handle GPIO change events or user-defined events.
    // You subscribe for events in the onStart() method (for example), so this will be called
    // when an event is posted to the App's event queue. The Eventmanager operates as a true
    // publish-subscribe system, so you can post events from anywhere in the application
    // and they will be delivered to ALL subscribers that have registered for that event type.

    if (e.notification.kind == NotificationKind::System)
    {
        switch (e.notification.system)
        {
            case SystemNotification::GpioChange:
            {
                printf("[App] GpioChange received\n");
                printf("[App] Pin = %u, Edge = 0x%X\n", e.gpioEvent.pin, e.gpioEvent.edge);

                const GpioEvent &data = e.gpioEvent;
                printf("[App] GPIO changed - pin %u: %s\n",
                       data.pin,
                       (data.edge & GPIO_IRQ_EDGE_RISE) ? "rising" : (data.edge & GPIO_IRQ_EDGE_FALL) ? "falling" : "unknown");
                break;
            }

            case SystemNotification::NetworkReady:
                std::cout << "[App] Network ready. Starting services..." << std::endl;
                pico.onNetworkReady(); // Notify the PicoModel that the network is ready
                printf("[App] Network ready. Starting HTTP server...\n");
                server.start();
                printf("[App] HTTP server started\n");
                break;

            case SystemNotification::TimeValid:
                std::cout << "[App] Time is valid. Scheduler can be initialized here." << std::endl;
                // scheduler.start();  // <- placeholder for future logic
                break;
            
                case SystemNotification::TimeSync:
                std::cout << "[App] SNTP Time Sync event." << std::endl;
                // no need to do anything here, the time is valid - these occur every hour
                break;

            case SystemNotification::TimeInvalid:
                std::cout << "[App] Time is invalid. Running in degraded mode." << std::endl;
                // handle degraded state if needed
                break;

            default:
                break;
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
    static int count = 0;
    if (count == 0) {
        printf("[App] Starting main polling loop...\n");
    }
    count++;
    vTaskDelay(pdMS_TO_TICKS(100)); // Yield to other tasks
    // This function is called continuously (non-blocking) from the App task.
    // Use RUN_EVERY to do periodic background work:

    runEvery(15000, [&]()
             { 
            printf("[App] Running main polling loop...\n"); 
            Event userEvt(static_cast<uint8_t>(UserNotification::Heartbeat));
            AppContext::get<EventManager>()->postEvent(userEvt);
            // this yield is not essential, but without yielding you can miss events
            vTaskDelay(pdMS_TO_TICKS(1)); }, "logLoop"); // <-- Unique ID for this timer (enables it to be cancelled)
}
