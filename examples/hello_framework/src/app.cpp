#include "app.h"
#include <iostream>
#include "events/Notification.h"
#include "http/Router.h"

App::App(int port) : FrameworkApp(port, "AppTask", 2048, 1) {
    std::cout << "App constructed" << std::endl;
}

void App::initRoutes() {
    router.addRoute("GET", "/", [this](HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
        req.printHeaders();
        res.send("Hello from Ian Archbell!");
    });
    // Add more routes as needed
}

void App::onStart() {
    std::cout << "[App] Waiting for network..." << std::endl;
    
    waitFor(SystemNotification::NetworkReady);

    std::cout << "[App] Network ready. Building routing table..." << std::endl;

    if (server.start()) {
        std::cout << "[App] HTTP server started!" << std::endl;
    } else {
        std::cerr << "[App] Failed to start HTTP server." << std::endl;
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
