#include "app.h"
#include <iostream>
App::App(int port) 
    : server(port, router)
{
        // Initialize the routes
        initRoutes();
}

void App::initRoutes() {

    router.addRoute("GET", "/", [this](Request &req, Response &res, const std::vector<std::string> &params) {
        req.printHeaders();
        res.send("Hello from Ian Archbell");
    });
}

void App::run() {
    if (server.start()) {
        std::cout << "HTTP server started successfully!" << std::endl;
    } else {
        std::cerr << "Failed to start HTTP server!" << std::endl;
    }
    while (true) {
        // put your application logic here
        // e.g. start your controller
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}