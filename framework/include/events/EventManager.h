/**
 * @file EventManager.h
 * @author Ian Archbell
 * @brief Event pub/sub manager for embedded applications using FreeRTOS.
 *
 * Part of the PicoFramework application framework.
 * This module provides the EventManager class, which allows tasks to subscribe
 * to specific event types and post events to a global queue.
 *
 * The EventManager uses a FreeRTOS queue to manage events and supports
 * both task and ISR contexts for posting events.
 *
 * Tasks can subscribe to specific event types using a bitmask and receive notifications
 * via `onEvent()` or FreeRTOS task notifications. It is possible to broadcast events
 * to all subscribers or target specific tasks based on their subscriptions.
 *
 * This design allows for flexible event-driven programming in embedded systems,
 * enabling modular and decoupled task management.
 *
 * The EventManager is implemented as a singleton, ensuring that there is
 * only one global event queue shared across the application.
 *
 * @note Events are represented by the `Event` class, which encapsulates the event type
 * and any associated data. You can extend the `Event` class to include additional
 * information as needed for your application.
 *
 * @note This is designed to be used in conjunction with the FrameworkTask class,
 * which provides the `onEvent()` method for handling events.
 *
 * @version 0.2
 * @date 2025-04-22
 *
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H
#pragma once

#include <vector>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include "events/Event.h"
#include "framework/FrameworkController.h"

// Forward declaration
class FrameworkTask;

/**
 * @brief Manages the system-wide event queue and subscriptions.
 *
 * Allows tasks to:
 * - Subscribe to specific event types (via bitmask)
 * - Post events (from task or ISR context)
 * - Receive and process events via `onEvent()`
 *
 * Implemented as a singleton.
 */
class EventManager
{
public:
    /**
     * @brief Get the global EventManager instance.
     */
    static EventManager &getInstance()
    {
        static EventManager instance;
        return instance;
    }

    /**
     * @brief Constructor with optional queue size override.
     *
     * @param queueSize Maximum number of events in the internal queue.
     */
    explicit EventManager(size_t queueSize = 0); // Optional override

    /**
     * @brief Subscribe a task to specific event types.
     *
     * Each controller provides a bitmask of events it is interested in.
     *
     * @param eventMask Bitmask of `(1 << event.notification.code())`.
     * @param controller Pointer to the FrameworkController to notify.
     * 
     * @note All EventManager::subscribe() for thread safety, calls must complete before interrupts are enabled."
     */
    void subscribe(uint32_t eventMask, FrameworkController *controller);


    /**
     * @brief Post a notification to the queue and notify matching subscribers.
     *
     * Safe to call from task or ISR context.
     *
     * @param n The notification to post.
     * @param target Optional specific controller to notify (nullptr for all).
     */
    void postEvent(const Event& e);

    /**
     * @brief Post a notification to the queue and notify matching subscribers.
     *
     * Safe to call from task or ISR context.
     *
     * @param n The notification to post.
     * @param target Optional specific controller to notify (nullptr for all).
     */
    void postNotification(const Notification& n, FrameworkTask* target);

    /**
     * @brief Post an event to the queue and notify matching subscribers.
     *
     * Safe to call from task or ISR context.
     *
     * @param event The event to post.
     */
    void enqueue(const Event &event);

    /**
     * @brief Returns true if there are any pending events for a given controller.
     *
     * @param controller Pointer to the controller.
     * @return True if any matching events exist.
     */
    bool hasPendingEvents(FrameworkController *controller) const;

private:
    struct Subscriber
    {
        uint32_t eventMask;              ///< Bitmask of subscribed event codes
        FrameworkController *controller; ///< Target controller to notify
    };

    SemaphoreHandle_t lock;
    static StaticSemaphore_t lockBuffer_;

    std::vector<Subscriber> subscribers_;
    
    void withSubscribers(const std::function<void(std::vector<Subscriber>&)>& fn);
    /**
     * @brief Provides read-only access to subscribers from ISR context (no locking).
     *
     * WARNING: Call only from ISR context. Must not modify the list.
     */
    void withSubscribersFromISR(const std::function<void(std::vector<Subscriber>&)>& fn);

};

#endif // EVENT_MANAGER_H
