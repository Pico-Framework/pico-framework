/**
 * @file FrameworkApp.cpp
 * @author Ian Archbell
 * @brief Implementation of FrameworkApp for embedded applications.
 *
 * Part of the PicoFramework application layer.
 * This module defines the FrameworkApp class, which serves as a base for
 * embedded applications using the PicoFramework. It integrates task management,
 * HTTP server functionality, routing, and framework services.
 *
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "framework/FrameworkApp.h"
#include "framework/FrameworkManager.h" // <-- This is important for wiring up system services, right now that means starting WiFi and setting the system time from a network time server

/// @copydoc FrameworkApp::FrameworkApp
FrameworkApp::FrameworkApp(int port, const char *name, uint16_t stackSize, UBaseType_t priority)
    : FrameworkController(name, router, stackSize, priority),
      router(),
      server(port, router),
      manager(this, router)
{
    manager.start(); 
}

/// @copydoc FrameworkApp::start
void FrameworkApp::start()
{
    FrameworkController::start(); // Start this app's task and calls run(), you are safely in the FreeRTOS task context in run())
}

void FrameworkApp::onStart()
{
    FrameworkController::onStart(); // Call base class start logic (including any route initialization)
}