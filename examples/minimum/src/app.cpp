
/**
* @file app.cpp
* @author Ian Archbell
* @brief Minimal PicoFramework application showcasing routing and request/response usage.
* @version 0.1
* @date 2025-05-19
* @license MIT
* @copyright Copyright (c) 2025, Ian Archbell
*
* This is the simplest route example, with the minimum of comments so you can see the structure clearly.
* See the hello_framework example for a heavily commented example that focuses solely on routing.
*/

#include "app.h"
#include <iostream>
#include "events/Notification.h"
#include "events/EventManager.h"
#include "http/Router.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "framework/AppContext.h"

App::App(int port) : FrameworkApp(port, "AppTask", 2048, 1)
{
    std::cout << "App constructed" << std::endl;
}

/**
 * @brief Initializes the HTTP routes for the application.
 * This method sets up the routing table for the HTTP server.   
 */
void App::initRoutes()
{
    router.addRoute("GET", "/", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        req.printHeaders();
        res.send("Hello from Ian Archbell!");
    });

    // Add more routes as needed
}

/**
 * @brief This method is called by the framework when the application starts.
 * At this point the FreeRTOS task is running and the application is ready to process events.
 * It is a good place to initialize resources, start services, etc.
 */
void App::onStart()
{
    std::cout << "[App] Waiting for network..." << std::endl;

    AppContext::getInstance().get<EventManager>()->subscribe(eventMask(SystemNotification::NetworkReady), this);

    waitFor(SystemNotification::NetworkReady);

    std::cout << "[App] Network ready. Building routing table..." << std::endl;

    server.start();
}

/**
 * @brief This method is called when an event is received.
 */
void App::onEvent(Event &e)
{
    if (e.notification.kind == NotificationKind::System)
    {
        switch (e.notification.system)
        {

        case SystemNotification::NetworkReady:
            // Start the HTTP server here in non-blocking mode
            break;

        case SystemNotification::TimeValid:
            std::cout << "[App] Time is valid. Your scheduler, if using one, can be initialized here." << std::endl;
            // If you rely on calendar time, you can start your scheduler, server ro anything else that relies on time
            break;

        default:
            // event we are not handling
            break;
        }
    }
}

/**
 * @brief Called periodically by the framework. No polling tasks are defined in this example.
 */
void App::poll()
{
    // For example, you can check the status of sensors, etc.
    runEvery(1000, []()
    {
        // This will execute once every 1000ms
        std::cout << "[App] Polling..." << std::endl;
    });
}
