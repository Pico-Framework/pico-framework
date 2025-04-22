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

#pragma once

#include "FrameworkTask.h"
#include <unordered_map>
#include <functional>
#include <string>
#include "FreeRTOS.h"
#include "task.h"
#include "Event.h"
#include "Router.h"

/**
 * @brief Base class for event-driven control logic in embedded applications.
 *
 * FrameworkController builds on FrameworkTask to offer structured hooks:
 * - `onStart()` – called once at task start
 * - `onEvent()` – called when an event is received
 * - `poll()` – called periodically for background work
 *
 * It also includes a utility `runEvery()` to call a function at regular intervals.
 */
class FrameworkController : public FrameworkTask
{
public:
    /**
     * @brief Constructor.
     *
     * @param name Task name.
     * @param stackSize Stack size in words.
     * @param priority Task priority (default is one above idle).
     */

/**
     * @brief Constructor for framework controllers.
     * 
     * @param name Task name.
     * @param sharedRouter Reference to the shared Router instance.
     * @param stackSize Task stack size (in words).
     * @param priority Task priority.
     */
    FrameworkController(const char* name, Router& sharedRouter, uint16_t stackSize = 1024, UBaseType_t priority = tskIDLE_PRIORITY + 1);

    /**
     * @brief Main task loop.
     *
     * Calls `onStart()` once, then enters a loop that:
     * - Waits for an event (or timeout)
     * - Dispatches it to `onEvent()`
     * - Calls `poll()` to perform non-blocking logic
     */
    void run() override final;

    /**
     * @brief Get the name of this controller.
     *
     * @return Controller name.
     */
    const char *getName() const { return FrameworkTask::getName(); }

protected:
    /**
     * @brief Called once at task start before entering the main loop.
     *
     * Override this to initialize controller state.
     */
    virtual void onStart();

    /**
     * @brief Initialize routes for this controller.
     *
     * Override to define HTTP routes or other event handlers.
     * This is called once at task start.
     */
    virtual void initRoutes();

    /**
     * @brief Called when an event is dispatched to this controller.
     *
     * Override to implement your event-driven logic.
     */
    virtual void onEvent(const Event &event);

    /**
     * @brief Called during every loop iteration for non-blocking background logic.
     *
     * Runs after waitAndDispatch() — useful for polling sensors or internal FSMs.
     */
    virtual void poll();

    /**
     * @brief Run a function periodically with millisecond resolution.
     *
     * @param intervalMs Time interval in milliseconds.
     * @param fn Function to run.
     * @param id Unique ID for this timed function (used to track last execution).
     */
    void runEvery(uint32_t intervalMs, const std::function<void()> &fn, const char *id);
    void runEvery(uint32_t intervalMs, const std::function<void()> &fn);

    Router& router;    ///< Handles path-to-handler mapping - reference to shared Router instance

public:

    // --- Event queue management ---
    /**
     * @brief Enable the event queue for this controller.
     *
     * Creates a FreeRTOS queue to hold events if it doesn't already exist.
     * The queue depth can be specified, defaulting to 8.
     *
     * @param depth Maximum number of events in the queue.
     */
    void enableEventQueue(size_t depth = 8) {
        if (!eventQueue_) {
            eventQueue_ = xQueueCreate(depth, sizeof(Event));
        }
    }
    /**
     * @brief Get the event queue for this controller.
     *
     * Returns the FreeRTOS queue handle used to hold events.
     * This queue is used to post events from other tasks or ISRs.
     */
    QueueHandle_t getEventQueue() const {
        return eventQueue_;
    }

    /**
     * @brief Check if there are any pending events in the queue.   
     * @param controller Pointer to the controller to check.
     * @return True if there are pending events, false otherwise.
     */
    bool getNextEvent(Event& event, uint32_t timeoutMs) {
        return eventQueue_ && xQueueReceive(eventQueue_, &event, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
    }


private:
    std::unordered_map<std::string, TickType_t> _timers; ///< Stores last-execution timestamps per ID
    
    QueueHandle_t eventQueue_ = nullptr; // FreeRTOS queue for pending events

    /**
     * @brief Waits for an event and dispatches it to `onEvent()` if applicable.
     *
     * If the event is targeted at this controller (or has no target), it will be passed to `onEvent()`.
     *
     * @param timeoutMs Maximum time to wait for an event in milliseconds.
     */
    void waitAndDispatch(uint32_t timeoutMs = portMAX_DELAY);
};

// --- Timing convenience macro ---

/**
 * @brief Utility macro for periodic polling using a unique ID.
 *
 * Expands to `runEvery(ms, fn, "function:line")` automatically.
 */
#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)
#define RUN_EVERY(ms, fn) runEvery(ms, fn, __FUNCTION__ ":" STRINGIFY(__LINE__))