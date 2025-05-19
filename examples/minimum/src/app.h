#ifndef APP_H
#define APP_H

#include "framework/FrameworkApp.h"

class App : public FrameworkApp {
public:
    App(int port);  // Constructor to initialize the app with a specific port

    /**
     * @brief Initializes the HTTP routes for the application.
     * This method sets up the routing table for the HTTP server.   
     */
    void initRoutes() override;
    /**
     * @brief Handles the start event of the application.
     * This method is called by the framework when the application starts.
     * At this point the FreeRTOS task is running and the application is ready to process events.
     */
    void onStart() override;
    /**
     * This method is called when an event is received.
     */
    void onEvent(Event& event);
    /**
     * The framework suppports both polling and event driven architectures
     */
    void poll() override; 
   
};

#endif
