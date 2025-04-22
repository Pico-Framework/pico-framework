/**
 * @file FrameworkController.h
 * @author Ian Archbell
 * @brief The FrameworkController class for event-driven control logic in embedded applications.
 * 
 * Extends FrameworkTask to provide a convenient structure for application logic
 * with periodic polling, event handling, and timed function execution.
 * 
 * Your application will typically implement one or more controllers to manage
 * different aspects of your system. Each controller can handle its own events,
 * perform background tasks, and run periodic functions.
 * 
 * You will typically use the EventManager, which provides pub/sub capabilities 
 * to dispatch events to the appropriate controller. The architucture allows for both event 
 * handling and polling in the same controller
 * 
 * This class is designed to be flexible and modular, allowing you to
 * create multiple instances for different parts of your application.
 * For example, you might have a `SensorController` for reading sensor data,
 * a `NetworkController` for handling communication, and a `MainController`
 * to coordinate overall application behavior.
 * 
 * Intended to be subclassed. You override `onStart()`, `onEvent()`, and `poll()`
 * to define your application's behavior.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "FrameworkController.h"
#include "EventManager.h"
#include "FrameworkTask.h"
#include "Event.h"
#include "AppContext.h"
#include "Router.h"
#include "GpioEvent.h" // temporary include for testing

FrameworkController::FrameworkController(const char* name, Router& sharedRouter, uint16_t stackSize, UBaseType_t priority)
    : FrameworkTask(name, stackSize, priority),
      router(sharedRouter) {}

/// @copydoc FrameworkController::run
void FrameworkController::run() {
    printf("Starting controller: %s\n", getName());
    enableEventQueue();  // ← MUST be here to initialize queue before use
    onStart();
    while (true) {
        waitAndDispatch(100);  // Wait for notifications or timeout
        poll();                // Call user logic
    }
}

/// @copydoc FrameworkController::onStart
void FrameworkController::onStart()
{
    initRoutes(); // Call base class start logic (including any route initialization)
}

/// @bcopydoc FrameworkController::initRoutes
void FrameworkController::initRoutes()
{
    // Default implementation does nothing
    // Override in derived classes to set up routes
}

/// @copydoc FrameworkController::onEvent
void FrameworkController::onEvent(const Event& event) {
    // Default: do nothing
}

/// @copydoc FrameworkController::poll
void FrameworkController::poll() {
    // Default no-op
}

/// @copydoc FrameworkController::waitAndDispatch
void FrameworkController::waitAndDispatch(uint32_t timeoutMs) {
    Event event;
    if (getNextEvent(event, timeoutMs)) {
        printf("[FrameworkController] Event received: %d\n", event.type);
        printf("[FrameworkController] Event target: %s\n", event.target ? event.target->getName() : "null");

        if (event.type == EventType::GpioChange) {
            printf("[FrameworkController] Event data: %p\n", event.data);
            printf("GpioEvent pin: %d, edge: %d\n", 
                   event.data ? static_cast<const GpioEvent*>(event.data)->pin : -1,
                   event.data ? static_cast<const GpioEvent*>(event.data)->edge : -1);  
        }

        // No longer need to filter by target — this controller owns this queue
        onEvent(event);
    }
}


/// @copydoc FrameworkController::runEvery
void FrameworkController::runEvery(uint32_t intervalMs, const std::function<void()>& fn, const char* id) {
    TickType_t now = xTaskGetTickCount();
    TickType_t& last = _timers[std::string(id)];

    if ((now - last) >= pdMS_TO_TICKS(intervalMs)) {
        fn();
        last = now;
    }
}
/// @copydoc FrameworkController::runEvery
void FrameworkController::runEvery(uint32_t intervalMs, const std::function<void()>& fn) {
    static int counter = 0;
    std::string generatedId = "auto_" + std::to_string(counter++);
    runEvery(intervalMs, fn, generatedId.c_str());
}

