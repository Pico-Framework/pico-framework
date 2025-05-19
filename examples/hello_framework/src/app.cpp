/**
 * @file app.cpp
 * @author Ian Archbell
 * @brief Minimal PicoFramework application showcasing routing and request/response usage.
 * @version 0.1
 * @date 2025-05-19
 * @license MIT
 * @copyright Copyright (c) 2025, Ian Archbell
 *
 * This example demonstrates how to use the PicoFramework to create a simple web application.
 * It shows how to define routes, handle HTTP requests and responses, and work with system-level events.
 *
 * All applications derive from the FrameworkApp class, which itself inherits from FrameworkController.
 * - FrameworkTask provides a thin wrapper around FreeRTOS task creation and scheduling.
 * - FrameworkController adds route initialization, event-handling and polling capabilities. Most real applications
 *   will have one or more of controllers and is the core capabality enabling you to use MVC or MVP patterns.
 * - FrameworkApp acts as a special kind of controller that owns the HTTP router and server,
 *   and is responsible for initializing the application. As it is a controller, it can define routes and handle events.
 *
 * Applications are launched by the framework. The framework creates the task automatically via FrameworkManager,
 * which also starts the network stack, sets up the AppContext service registry,
 * and emits system events such as NetworkReady and TimeValid.
 *
 * AppContext allows components to access shared services such as the EventManager (used in this example),
 * JsonService, and persistent storage interfaces.
 * 
 */

#include "app.h"
#include <iostream>
#include "events/Notification.h"
#include "events/EventManager.h"
#include "http/Router.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "framework/AppContext.h"

/**
 * @brief Constructor for the App class.
 * @param port The port number for the HTTP server.
 * This constructor initializes the app with a specific port and sets up the task name and stack size.
 * The task name is used for debugging purposes and the stack size is used to allocate memory for the task.
 * The task is created by the framework and the app is started by the framework.
 * The constructor does not start the app, it just initializes the app and sets up the task.
 * You should not peform any blocking operations in the constructor, as it will block the task creation.
 * You should use the onStart method to perform any initialization that requires the task to be running.
 * The onStart method is called by the framework when the app is started.
 * Do not use the FreeRTOS API in the constructor, as it may not be safe to do so as the task is not yet running
 * and the FreeRTOS scheduler is not yet started.
 */
App::App(int port) : FrameworkApp(port, "AppTask", 2048, 1)
{
    std::cout << "App constructed" << std::endl;
}

/**
 * @brief Initializes the HTTP routes for the application.
 * This method sets up the routing table for the HTTP server.
 * The routes are defined using the router object, which is a member of the App class.
 * The routes are defined using lambda functions that take a request and response object as parameters.
 * The routes can handle different HTTP methods (GET, POST, PUT, DELETE) and can have parameters in the URL.
 * The routes can also return JSON data or plain text responses.
 * initRoutes is called by the framework when the app is started but before the task is running.
 * Both the router and server are dependency injected in Framework::App, so both are available to App.
 */
void App::initRoutes()
{
    /**
     * @brief Simplest route, just returns a string.
     * In other routes below we will use the req and res objects to process the request and send an appropriate response.
     * You can send a string, JSON, binary data, etc. You can also serve static files, images, etc. using the router.
     * The req object contains the request data, headers, body, etc. The res object is used to send the response back
     * to the client. You can set the status code, headers, body, etc. Us ethe browser at http://Pico-Framework or the ip address
     * of the device to see the response. You willl need to use a tool like Postman or curl to test POST, PUT or DELETE routes.
     */
    router.addRoute("GET", "/", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        req.printHeaders();
        res.send("Hello from Ian Archbell!");
    });

    /**
     * @brief This is the syntax for a route with a parameter; multiple parameters are supported (e.g. /user/{id}/status/{status}).
     * Parameters are passed as a map to the route handler. The parameter name is the key and the value is the value of the parameter.
     * For example, if the route is /{name} and the request is /John, the parameter name will be "name" and the value will be "John".
     */
    router.addRoute("GET", "/{name}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        std::string name = match.getParam("name").value_or("World");
        res.send("Hello " + name + "!"); 
    });

    /**
     * @brief Returns mocked JSON data. In later examples we will use JsonService to persist data.
     */
    router.addRoute("GET", "/api/data", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        res.json({{"key", "value"}, {"number", 42}}); 
    });

    /**
     * @brief Accepts raw POST body data in any format (e.g. JSON, form data, plain text) and echoes it back.
     * If using JSON, you can use req.json() to parse it. If using form data, use req.getFormParams().
     * Use postman or curl to send a POST request to this endpoint with a body.
     */
    router.addRoute("POST", "/submit", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        std::string data = req.getBody();
        res.send("Data received: " + data); 
    });

    /**
     * @brief Accepts JSON data via POST. Returns an error if invalid. Otherwise echoes the parsed JSON.
     * You can also access the raw body using req.getBody().
     * You can use postman or curl to send a JSON body to this endpoint.
     * e.g. {"key": "value", "number": 42}
     */
    router.addRoute("POST", "/api/json", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        auto json = req.json();
        if (json.is_discarded()) {
            // If the JSON is invalid, we will return a 400 error, you might have a success, true or false response
            // depending on your API design
            res.status(400).json({{"error", {{"code", "INVALID_JSON"}, {"message", "The body must be valid JSON"}}}});
            return;
        }
        res.json({{"received", json}}); 
    });

    /**
     * @brief Accepts URL-encoded form data via POST (application/x-www-form-urlencoded).
     * Returns 400 if empty. Echoes the parsed form parameters as JSON.
     */
    router.addRoute("POST", "/api/form", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        auto formParams = req.getFormParams();
        if (formParams.empty()) {
            res.status(400).json({{"error", "Invalid Form Data"}});
            return;
        }
        res.json({{"received", formParams}}); 
    });

    /**
     * @brief Accepts query string parameters (?key=value). Returns 400 if none found. 
     * Echoes the parsed query parameters as JSON. Query data comes from the URL query string (?key=value)
     * e.g. /api/query?param1=value1&param2=value2
     */
    router.addRoute("GET", "/api/query", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        auto queryParams = req.getQueryParams();
        if (queryParams.empty()) {
            res.status(400).json({{"error", "Invalid Query String"}});
            return;
        }
        res.json({{"received", queryParams}}); 
    });

    /**
     * @brief Demonstrates use of PUT method with dynamic path parameter `{id}`.
     * This is a common pattern for REST APIs where you want to update a resource identified by an ID.
     * The ID is passed as a path parameter and can be accessed using the match object.
     */
    router.addRoute("PUT", "/update/{id}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        auto id = match.getParam("id").value_or("unknown");
        res.send("PUT request for ID: " + id); 
    });
    
    /**
     * @brief Demonstrates use of DELETE method with dynamic path parameter `{id}`.
     * This is a common pattern for REST APIs where you want to delete a resource identified by an ID.
     * The ID is passed as a path parameter and can be accessed using the match object.
     */
    router.addRoute("DELETE", "/delete/{id}", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        auto id = match.getParam("id").value_or("unknown");
        res.send("DELETE request for ID: " + id); 
    });

    /**
     * @brief Returns the value of the "User-Agent" header using req.getHeader().
     * If the header is not present, returns "Unknown".
     * You can use req.getHeaders() to get all the headers as a map and process them
     */
    router.addRoute("GET", "/api/header", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &)
    {
        auto userAgent = req.getHeader("User-Agent");
        res.json({{"user-agent", userAgent}}); 
    });

    /**
     * @brief Returns the full request headers map and extracts "User-Agent" manually.
     * Demonstrates use of req.getHeaders().
     */
    router.addRoute("GET", "/api/headers", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &)
    {
        std::string userAgent;
    
        auto it = req.getHeaders().find("User-Agent");
        if (it != req.getHeaders().end()) {
            userAgent = it->second;
        } else {
            userAgent = "Unknown";
        }
    
        res.json({
            {"user-agent", userAgent},
            {"all-headers", req.getHeaders()}
        }); 
    });
    
    /**
     * @brief Returns a custom header ("X-Custom-Header") and a 202 Accepted status code.
     */
    router.addRoute("GET", "/api/custom", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &)
    {
        // This route will return a custom header
        res.setHeader("X-Custom-Header", "PicoFramework").status(202).send("Accepted but not processed"); 
    });

    /**
     * @brief Catch-all route for unmatched GET requests. Returns 404 Not Found.
     * Note: This does not apply to POST/PUT/DELETE. Those methods will return 404 by default if unmatched.
     * We deliberately do not provide a catch-all for other methods as it may be unsafe.
     * 
     * You can use it to serve static files, redirect to a default page, etc.
     * In this simple example, we will just return a 404.
     */
    router.addCatchAllGetRoute([this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        res.status(404).send("Not Found"); 
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

    // Subscribe to the network ready event
    // This will allow us to handle the event when the network is ready
    // The event manager is a singleton that manages events and notifications
    // Once the network is ready, we will start the HTTP server
    AppContext::getInstance().get<EventManager>()->subscribe(eventMask(SystemNotification::NetworkReady), this);

    // This is a blocking call, waiting for the network to be ready
    // In most applications you probably want to use a non-blocking appraoch (see below)
    // and handle the event in the onEvent method, both choices are available because
    // the framework both notifies you of system events and posts them to the event queue
    waitFor(SystemNotification::NetworkReady);

    std::cout << "[App] Network ready. Building routing table..." << std::endl;

    // Both the router and server are dependecy injected in Framework::App
    // so both are available here
    if (server.start())
    {
        std::cout << "[App] HTTP server started!" << std::endl;
    }
    else
    {
        std::cerr << "[App] Failed to start HTTP server." << std::endl;
    }

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
/**
 * @brief This method is called when an event is received in the FrameworkController event queue.
 * There is one event queue per controller.
 * It is a good place to handle events that are not directly related to HTTP requests.
 * For example, you can handle system events, user events etc.
 * The onEvent method is called by the framework when an event is received..
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
}

/**
 * @brief This method is called to get the poll ticks.
 * It is used to determine how often the poll() method should be called.
 * @param ticks The number of ticks to wait before calling poll() again.
 * 
 * It is optional, the default is 100ms.
 * 
 * @note In general I'd encourage you to use a separate task if you have tight polling requirements as
 * the waitAndNotify() method handles the task notification and event queue.
 * 
 * In an MVC/MVP architecture you should be using the controller as a controller or presenter and using
 * a separate model to handle the data and business logic. In the case of embedded systems this is often
 * the task that interfaces with the hardware.
 */
TickType_t App::getPollIntervalTicks()
{
    return pdMS_TO_TICKS(100); // Default: 100ms
}