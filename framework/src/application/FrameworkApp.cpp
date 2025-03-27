/**
 * @file FrameworkApp.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "FrameworkApp.h"
#include "FrameworkManager.h"  // <-- this is critical!

FrameworkApp::FrameworkApp(int port, const char* name, uint16_t stackSize, UBaseType_t priority)
    : FrameworkTask(name, stackSize, priority),
      server(port, router),
      manager(new FrameworkManager(this)) {}


void FrameworkApp::start() {
    manager->start();           // Start network + any other tasks
    FrameworkTask::start();    // Launch the app's task (calls run())
}