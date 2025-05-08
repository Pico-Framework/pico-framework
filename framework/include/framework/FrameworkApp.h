/**
 * @file FrameworkApp.h
 * @author Ian Archbell
 * @brief Base class for applications using the PicoFramework.
 *
 * FrameworkApp inherits from FrameworkTask to provide task management,
 * HTTP server integration, and route initialization. Designed to be
 * subclassed by specific application implementations.
 *
 * Derived classes implement:
 * - `start()` to initialize the application
 * - `initRoutes()` to define route handlers
 * - `run()` to implement the main application loop
 *
 * @version 0.1
 * @date 2025-03-26
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#ifndef FRAMEWORK_APP_H
#define FRAMEWORK_APP_H
#pragma once

#include "framework/FrameworkManager.h"
#include "framework/FrameworkController.h"
#include "framework/AppContext.h"
#include "http/Router.h"
#include "http/HttpServer.h"

/**
 * @brief Base class for applications using the framework.
 *
 * FrameworkApp is designed to be subclassed by user applications.
 * It wires together the HTTP server, routing system, and task management.
 * Derived classes override `initRoutes()` to register route handlers,
 * and `run()` to implement application logic.
 */
class FrameworkApp : public FrameworkController
{
public:
    /**
     * @brief Constructor.
     *
     * Initializes the HTTP server with the given port and binds the router.
     * The router is passed to the server constructor so it can handle
     * incoming requests based on the defined routes.
     *
     * You can choose to call `initRoutes()` in the constructor of your subclass
     * to ensure routes are set up before the server starts.
     * This keeps the constructor focused on initialization and ensures
     * routing is handled in one place.
     *
     * @param port The TCP port to listen on.
     * @param name The name of the FreeRTOS task.
     * @param stackSize Stack size allocated to the task.
     * @param priority Task priority.
     */
    FrameworkApp(int port, const char* name = "AppTask", uint16_t stackSize = 2048, UBaseType_t priority = tskIDLE_PRIORITY + 1);

    /**
     * @brief Virtual destructor.
     *
     * Allows derived classes to clean up resources.
     */
    virtual ~FrameworkApp() = default;

    /**
     * @brief Initializes the application and its framework services.
     *
     * This method is responsible for setting up the application and preparing
     * it to handle incoming requests. It usually includes:
     * - Starting the FrameworkManager (network and service initialization)
     * - Setting up logging or middleware
     * - Any other application-specific initialization
     *
     * It is typically called before `run()` to ensure the application
     * is fully ready before entering the main loop.
     */
    virtual void start();
    /**
     * @brief Starts the Framework Manager.
     * This is called by the FrameworkApp::start() method once the FreeRTOS task has been created
     * and is ready to run. It calls the base class onStart() method to ensure
     * that routes are initialized before the server starts accepting requests.
     */
    void onStart() override;

    /**
     * @brief Define the application's HTTP routes.
     *
     * You must implement this method in your subclass. Define your routes here
     * using `router.addRoute(...)`. This centralizes route logic and ensures
     * all endpoints are registered before the server runs.
     *
     * Example:
     * @code
     * router.addRoute("GET", "/", [](HttpRequest& req, HttpResponse& res, const std::vector<std::string>&) {
     *     res.send("Hello from root!");
     * });
     * @endcode
     *
     * Or bind member functions:
     * @code
     * router.addRoute("GET", "/status",
     *     std::bind(&MyApp::handleStatus, this, _1, _2, _3));
     * @endcode
     */

protected:
    virtual void initRoutes() {} // Default no-op
    // ⚠️ Construction order is critical:
    // router → server → manager — do not reorder these declarations for correct construction.
    Router router;              ///< Router instance for handling HTTP routes
    HttpServer server;          ///< Embedded HTTP server instance
    FrameworkManager manager;   ///< Responsible for launching system services and networking
};

#endif // FRAMEWORK_APP_H
