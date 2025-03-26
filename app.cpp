#include "app.h"
#include <iostream>
App::App(int port) 
    : server(port, router)
{
    // Initialize the routes in the constructor
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
    initRoutes();
}

void App::initRoutes() {
    
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
    
    router.addRoute("GET", "/", [this](Request &req, Response &res, const std::vector<std::string> &params) {
        req.printHeaders();
        res.send("Hello from Ian Archbell");
    });
}

void App::run() {

    // Start the HTTP server
    // This method initializes the server and starts listening for incoming requests.
    // The server will use the defined routes to handle requests
    // and send appropriate responses based on the HTTP method and path.
    // The server will run indefinitely, processing requests as they come in.
    // Note: You may want to add error handling or logging here
    // to monitor the server's status and any issues that arise.
    // For example, you could log the server's status to the console
    // or to a file for later analysis.

    if (server.start()) {
        std::cout << "HTTP server started successfully!" << std::endl;
    } else {
        std::cerr << "Failed to start HTTP server!" << std::endl;
    }
    while (true) {
        // Keep the server running
        // This loop ensures that the server continues to run
        // and process incoming requests.
        // You can add any additional logic here if needed,
        // such as monitoring server status or handling signals.
        // Note: The vTaskDelay function is used to yield control
        // to other tasks in a FreeRTOS environment.
        // This is important to prevent the server from blocking
        // other tasks and to allow the system to remain responsive.
        // The pdMS_TO_TICKS macro converts milliseconds to ticks,
        // which is the time unit used by FreeRTOS for task scheduling.
        // In this case, we are delaying for 1000 milliseconds (1 second)
        // to allow other tasks to run while the server is active.

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}