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
 * @version 0.1
 * @date 2025-03-26
 * 
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #ifndef EVENT_MANAGER_H
 #define EVENT_MANAGER_H
 #pragma once
 
 #include <vector>
 #include "FreeRTOS.h"
 #include "task.h"
 #include "queue.h"
 #include "Event.h"
 
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
 class EventManager {
 public:
     /**
      * @brief Get the global EventManager instance.
      */
     static EventManager& getInstance() {
         static EventManager instance;
         return instance;
     }

     /**
      * @brief Constructor with optional queue size override.
      * 
      * @param queueSize Maximum number of events in the internal queue.
      */
     explicit EventManager(size_t queueSize = 32);
 
     /**
      * @brief Subscribe a task to specific event types.
      * 
      * @param eventMask Bitmask of `(1 << static_cast<uint8_t>(EventType))`.
      * @param task Pointer to the FrameworkTask to notify.
      */
     void subscribe(uint32_t eventMask, FrameworkTask* task);
 
     /**
      * @brief Post an event to the queue and notify subscribers.
      * 
      * Can be safely called from task or ISR context.
      * 
      * @param event The event to post.
      */
     void postEvent(const Event& event);
 
     /**
      * @brief Retrieve the next event from the queue.
      * 
      * Used by tasks to process events (e.g., in loop).
      * 
      * @param event Reference to fill with the retrieved event.
      * @param timeoutMs Timeout in milliseconds (0 = non-blocking).
      * @return true if an event was received.
      */
     bool getNextEvent(Event& event, uint32_t timeoutMs = portMAX_DELAY) const;
 
     /**
      * @brief Returns true if there are any pending events in the queue.
      */
     bool hasPendingEvents() const;
 
 private:
     struct Subscriber {
         uint32_t eventMask;      ///< Bitmask of subscribed event types
         FrameworkTask* task;     ///< Target task to notify
     };
 
     std::vector<Subscriber> subscribers_;
     QueueHandle_t eventQueue_;   ///< FreeRTOS queue for pending events
 };
 
 #endif // EVENT_MANAGER_H
 