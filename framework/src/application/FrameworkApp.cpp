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

 #include "FrameworkApp.h"
 #include "FrameworkManager.h"  // <-- This is critical for wiring up system services
 
 /// @copydoc FrameworkApp::FrameworkApp
 FrameworkApp::FrameworkApp(int port, const char* name, uint16_t stackSize, UBaseType_t priority)
     : FrameworkTask(name, stackSize, priority),
       server(port, router),
       manager(new FrameworkManager(this)) {}
 
 /// @copydoc FrameworkApp::start
 void FrameworkApp::start() {
     manager->start();           // Start network + other core services
     FrameworkTask::start();    // Start this app’s task (calls run())
 }
 