#include "app.h"
#include <iostream>

App::App(int port)
    : FrameworkApp("AppTask", 2048, 1), port(port) {
    std::cout << "App constructor done" << std::endl;
}

void App::start() {
    std::cout << "App::start - creating server and manager..." << std::endl;

    server = std::make_unique<HttpServer>(port, router);
    manager = std::make_unique<FrameworkManager>(this);

    manager->start();
    FrameworkTask::start();  // Launch App as a FreeRTOS task
}

void App::initRoutes() {
    router.addRoute("GET", "/", [this](Request &req, Response &res, const std::vector<std::string> &params) {
        req.printHeaders();
        res.send("Hello from Ian Archbell!");
    });
}

void App::run() {
    std::cout << "App::run - waiting for notification..." << std::endl;
    waitForNotification(portMAX_DELAY);  // Wait for network ready signal

    std::cout << "App::run - received notification, initializing server..." << std::endl;
    initRoutes();

    if (server->start()) {
        std::cout << "HTTP server started!" << std::endl;
    } else {
        std::cerr << "Failed to start HTTP server." << std::endl;
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
