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
     * It is a good place to initialize resources, start services, etc.
     */
    void onStart() override;
    /**
     * This method is called when an event is received.
     * It is a good place to handle events that are not directly related to HTTP requests.
     * For example, you can handle system events, user events etc.
     */
    void onEvent(Event& event);
    /**
     * The framework suppports both polling and event driven architectures
     */
    void poll() override; 

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
     * 
     */
    TickType_t getPollIntervalTicks() override;
   
};

#endif
