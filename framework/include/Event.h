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
  * @brief Enumerates supported event types.
  * 
  * Extend this enum as needed for your application domain.
  */
 enum class EventType : uint8_t {
    None = 0,
    SysStartup,
    SysError,
    UserInput,
    UserCommand,
    GpioChange,
    // Extend as needed
};

/**
 * @brief Represents a framework event used for task messaging.
 *
 * Each event contains:
 * - A `FrameworkNotification` type (e.g., NetworkReady, Shutdown)
 * - An optional target task (nullptr = broadcast to all)
 * - An optional payload pointer for custom event data
 */


 struct Event {
    EventType type{};                   ///< For user-defined events
    FrameworkTask* target = nullptr;   ///< Receiver
    void* source = nullptr;            ///< Sender
    const void* data = nullptr;        ///< Payload
    size_t dataSize = 0;               ///< Payload size

    Event() = default;

    // System/framework event (legacy support)
    Event(FrameworkNotification frameworkType, FrameworkTask* target = nullptr, void* data = nullptr)
        : type(static_cast<EventType>(frameworkType)), target(target), data(data) {}

    // App/user event
    Event(EventType type, const void* data = nullptr, size_t size = 0, void* source = nullptr)
        : type(type), data(data), dataSize(size), source(source) {}
};

