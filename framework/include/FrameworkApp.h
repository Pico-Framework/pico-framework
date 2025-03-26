/**
 * @file FrameworkApp.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef FRAMEWORK_APP_H
#define FRAMEWORK_APP_H
#pragma once

#include "Router.h"
#include "HttpServer.h"
//#include "FrameworkManager.h" // Include the framework manager for task management
#include "FrameworkTask.h" // Include the base task class

class FrameworkManager;

class FrameworkApp : public FrameworkTask {

    // FrameworkApp is a base class for applications using the framework.
    // It inherits from FrameworkTask to provide task management capabilities.
    // This class is designed to be subclassed by specific application implementations.
    // The derived classes will implement the start(), initRoutes(), and run() methods
    // to define the application's behavior and routing logic.
public:
    //FrameworkApp(const char* name = "AppTask", uint16_t stackSize = 2048, UBaseType_t priority = 1);
    FrameworkApp(int port, const char* name, uint16_t stackSize = 2048, UBaseType_t priority = 1);

    // Call initRoutes() in the constructor to set up the routes
    // This ensures that the routes are set up before the server starts
    // and the application runs.
    // You can also choose to initialize routes in a separate method
    // if you prefer to keep the constructor clean.
    // However, for simplicity, we are calling initRoutes() here.
    // This allows you to easily modify the routes in one place
    // and keep the constructor focused on initializing the server.
    // Note: The router is passed to the server constructor,
    // so it can handle incoming requests based on the defined routes.
    // The server will use the router to match requests to the appropriate route handlers.
    // The router is a key component of the server, as it defines how   
    // the server responds to different HTTP methods and paths.
    // FrameworkApp() {
    //     initRoutes(); // Initialize routes in the constructor
    // }

    // Virtual destructor to allow derived classes to clean up resources
    virtual ~FrameworkApp() = default;

    // start() method to initialize the application
    // This method is responsible for setting up the application
    // and preparing it to handle incoming requests.
    // It generally will include calling the FrameworkManager's start method
    // to initialize the network and application tasks.
    // The start method may also include any other initialization logic
    // specific to your application, such as setting up logging,
    // configuring middleware, or initializing any other components.
    // The start method is typically called before the run() method
    // to ensure that the application is fully initialized
    // and ready to handle requests when the server starts.
    virtual void start(); // Initialize the application
    
    // add your routes here
    // Example: Adding a simple GET route
    // This route responds to GET requests at the root path "/"
    // and sends a simple text response.
    // You can replace this with your own logic or controller methods.
    // Note: The lambda function captures 'this' to access the class instance
    // and its members if needed.
    // You can also use member functions instead of lambdas if preferred.
    // For example, you could have a method like:
    // void handleRoot(Request &req, Response &res) { ... }
    // and then bind it like this:
    // router.addRoute("GET", "/", std::bind(&App::handleRoot, this, std::placeholders::_1, std::placeholders::_2));
    virtual void initRoutes() = 0;
   
    // Start the HTTP server
    // This method initializes the server and starts listening for incoming requests.
    // The server will use the defined routes to handle requests
    // and send appropriate responses based on the HTTP method and path.
    // The server will run indefinitely, processing requests as they come in.
    // Note: You may want to add error handling or logging here
    // to monitor the server's status and any issues that arise.
    // For example, you could log the server's status to the console
    // or to a file for later analysis.
    // then enter your unlimited loop to process requests
    // and keep the server running.
    virtual void run() = 0;

    protected:
        Router router;
        HttpServer server;
        FrameworkManager* manager;

};

#endif // FRAMEWORK_APP_H