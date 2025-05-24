/**
 * @file App.cpp
 * @brief Application class for the PicoFramework test application.
 * This class extends the FrameworkApp to provide application-specific functionality,
 * including route initialization, event handling, and test functions.
 * @version 0.1
 *  
 * @date 2025-04-14
 * 
 * @license MIT License
 * 
 * @copyright Copyright (c) 2025, Ian Archbell
 *  
 */

#ifndef APP_H
#define APP_H

#include "framework/FrameworkApp.h"
#include "ZoneModel.h"
#include "ProgramModel.h"
#include "SprinklerScheduler.h"
#include "SprinklerController.h"
#include "LogController.h"

class App : public FrameworkApp
{
public:
    App(int port); // Constructor to initialize the application with a port number

    // standard functions
    void initRoutes() override;            // called by the framework to initialize routes
    void onStart() override;               // called by the framework when the app is started
    void poll() override;                  // called by the framework to poll the app
    void onEvent(const Event &e) override; // called by the framework when an event occurs

private:
    ZoneModel zoneModel; // Model for managing zones
    ProgramModel  programModel; // Model for managing sprinkler programs
    SprinklerScheduler scheduler;  // Scheduler for managing sprinkler programs
    SprinklerController controller; // Controller for managing sprinkler zones
    LogController logController; // Controller for logging events and system messages
};

#endif