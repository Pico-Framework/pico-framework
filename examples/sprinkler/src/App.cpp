/**
 * @file App.cpp
 */

#include "App.h"
#include "framework/FrameworkApp.h"
#include "events/EventManager.h"
#include "events/Event.h"
#include "events/Notification.h"
#include "UserNotification.h"

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
    router.addRoute("GET", "/hello", [](HttpRequest &req, HttpResponse &res, const auto &)
                    { res.send("Welcome to PicoFramework!"); });
    
    router.addRoute("GET", "/ls", [](HttpRequest &req, HttpResponse &res, const auto &) {
                        std::vector<FileInfo> files;
                        AppContext::get<StorageManager>()->listDirectory("/", files);
                        res.json(files);
    });
                    
}

void App::onStart()
{
    // Call the base class to ensure the framework starts correctly.
    FrameworkApp::onStart();
    
    std::cout << "[App] Initializing application..." << std::endl;

    scheduler.setProgramModel(&programModel);
    controller.setZoneModel(&zoneModel);

    // Load persisted state
    zoneModel.load();
    programModel.load();

    // Start scheduler and register controller
    scheduler.start();

    // Post initial scheduling event
    Event startupEvent;
    startupEvent.notification.kind = NotificationKind::User;
    startupEvent.notification.user_code = static_cast<uintptr_t>(UserNotification::SchedulerCheck); // Define your enum
    AppContext::get<EventManager>()->postEvent(startupEvent);


    EventManager *eventManager = AppContext::get<EventManager>();
    eventManager->subscribe(
        eventMask(
            SystemNotification::NetworkReady,
            SystemNotification::TimeValid,
            SystemNotification::TimeSync,
            SystemNotification::TimeInvalid),
        this);
}

void App::onEvent(const Event &e)
{
    // This method is called when an event is posted to the App's event queue.
    // It is called by the FrameworkController base class when an event is received.
    // You can handle different types of events here based on the notification kind.
    // For example, you can handle system, GPIO change events or user-defined events.
    // You subscribe for events in the onStart() method (for example), so this will be called
    // when an event is posted to the App's event queue. The Eventmanager operates as a true
    // publish-subscribe system, so you can post events from anywhere in the application
    // and they will be delivered to ALL subscribers that have registered for that event type.

    if (e.notification.kind == NotificationKind::System)
    {
        switch (e.notification.system)
        {

        case SystemNotification::NetworkReady:
            std::cout << "[App] Network ready. Starting services..." << std::endl;
            break;

        case SystemNotification::TimeValid:
            std::cout << "[App] Time is valid. Scheduler can be initialized here." << std::endl;
            // we can start the HTTP server in NetworkReady
            // or TimeValid, but not before the network is up
            // We may use calendar time so we start the server here
            server.start();
            cyw43_arch_gpio_put(0,1); // indicate that the network is up
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
    // Handle other event types here
    // For example, you can check for GPIO change events or user-defined events.
}

void App::poll()
{
    static int count = 0;
    if (count == 0)
    {
        printf("[App] Starting main polling loop...\n");
    }
    count++;
    vTaskDelay(pdMS_TO_TICKS(100)); // Yield to other tasks
    // This function is called continuously (non-blocking) from the App task.
    // Use RUN_EVERY to do periodic background work:

    runEvery(15000, [&]()
             { 
            printf("[App] Running main polling loop...\n"); 
            // Yield to other tasks
            // This is where you can do non-blocking work, like checking sensors or updating state.
            // You can also use this to run periodic tasks.
            vTaskDelay(pdMS_TO_TICKS(1)); }, "logLoop"); // <-- Unique ID for this timer (enables it to be cancelled)
}
