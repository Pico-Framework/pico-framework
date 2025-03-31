/**
 * @file Event.h
 * @author Ian Archbell
 * @brief Event struct used in the framework event system.
 * 
 * Part of the PicoFramework application framework.
 * Represents a typed event optionally targeted at a specific task,
 * and optionally carrying a payload. Used by EventManager and the
 * framework's pub/sub notification system.
 * 
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #pragma once

 #include "FrameworkNotification.h"
 
 // Forward declaration to avoid circular include
 class FrameworkTask;
 
 /**
  * @brief Represents a framework event used for task messaging.
  * 
  * Each event contains:
  * - A `FrameworkNotification` type (e.g., NetworkReady, Shutdown)
  * - An optional target task (nullptr = broadcast to all)
  * - An optional payload pointer for custom event data
  */
 struct Event {
     FrameworkNotification type = FrameworkNotification::None;  ///< Type of event
     FrameworkTask* target = nullptr;                           ///< Target task, or nullptr for broadcast
     void* data = nullptr;                                      ///< Optional data payload
 
     /**
      * @brief Default constructor (type = None, no target, no data)
      */
     Event() = default;
 
     /**
      * @brief Construct a new Event.
      * 
      * @param type Notification type.
      * @param target Optional target task (nullptr = broadcast).
      * @param data Optional payload.
      */
     Event(FrameworkNotification type, FrameworkTask* target, void* data = nullptr)
         : type(type), target(target), data(data) {}
 
     /**
      * @brief Helper to create a broadcast event (no specific target).
      * 
      * @param type Notification type.
      * @param data Optional payload.
      * @return A new Event with no target.
      */
     static Event broadcast(FrameworkNotification type, void* data = nullptr) {
         return Event(type, nullptr, data);
     }
 };
 