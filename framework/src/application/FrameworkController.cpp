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

/// @copydoc FrameworkController::FrameworkController
FrameworkController::FrameworkController(const char* name, uint16_t stackSize, UBaseType_t priority)
    : FrameworkTask(name, stackSize, priority) {}

/// @copydoc FrameworkController::run
void FrameworkController::run() {
    onStart();
    while (true) {
        waitAndDispatch(100);  // Wait for notifications or timeout
        poll();                // Call user logic
    }
}

/// @copydoc FrameworkController::onStart
void FrameworkController::onStart() {
    // Default no-op
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
    if (EventManager::getInstance().getNextEvent(event, timeoutMs)) {
        if (event.target == nullptr || event.target == this) {
            onEvent(event);
        }
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
