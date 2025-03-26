#include "app.h"
#include <iostream>

App::App(int port) : server(port, router) {}

void App::initRoutes() {
    router.addRoute("GET", "/", [this](Request &req, Response &res, const std::vector<std::string> &params) {
        req.printHeaders();
        res.send("Hello from Ian Archbell!");
    });
}

void App::run() {
    if (server.start()) {
        std::cout << "HTTP server started!" << std::endl;
    } else {
        std::cerr << "Failed to start HTTP server." << std::endl;
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
