#include "App.h"
#include <iostream>
#include "events/Notification.h"
#include "framework/AppContext.h"
#include "http/HttpServer.h"
#include "StorageController.h"

App::App(int port) : FrameworkApp(port, "AppTask", 1024, 1) {
    
}

void App::onStart() {
    
    FrameworkApp::onStart();

    printf("[MyApp] Starting Storage App...\n");

    // Set up storage controller
    static StorageController storageController(routerInstance);
    storageController.start();
    printf("[MyApp] StorageController started.\n");

    // Wait for network
    printf("[MyApp] Waiting for network...\n");
    waitFor(SystemNotification::NetworkReady);

    // Start server once network is ready
    printf("[MyApp] Network ready. Starting HTTP server...\n");
    server.start();
}

void App::poll() {
    vTaskDelay(pdMS_TO_TICKS(100)); // Light yield to let other tasks run
}
