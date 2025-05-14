/**
 * @file Event.h
 * @brief Defines the Event structure and related utilities for event messaging.
 * @version 1.1
 * @date 2025-04-22
 * @copyright Copyright (c) 2025, Ian Archbell
 * @license MIT
 */

#pragma once
#include <cstdint>
#include <cstddef>
#include "events/Notification.h"
#include "events/GpioEvent.h"

class FrameworkTask;

/**
 * @brief Represents a framework event, optionally carrying payload data.
 */
struct Event
{
    Notification notification;       ///< Notification identifier (system or user)
    union {
        GpioEvent gpioEvent;         ///< Inline data if GpioChange
        const void* data;            ///< For user use
    };
    size_t size = 0;                 ///< Size of payload data  
    void *source = nullptr;          ///< Optional source (e.g. controller that generated the event)
    FrameworkTask *target = nullptr; ///< Optional specific target (for directed delivery)

    /**
     * @brief Default constructor (creates a SystemNotification::None event).
     */
    Event() = default;

    /**
     * @brief Construct an event with a system notification.
     *
     * @param type   System notification type
     * @param data   Optional pointer to payload
     * @param size   Payload size
     * @param source Optional source pointer
     * @param target Optional target task
     */
    Event(SystemNotification type, const void *data = nullptr, size_t size = 0,
          void *source = nullptr, FrameworkTask *target = nullptr)
        : notification(type), data(data), size(size), source(source), target(target) {}

    /**
     * @brief Construct an event with a GpioEvent payload.
     *
     * @param type       System notification type (must be SystemNotification::GpioChange)
     * @param gpioEvent  GpioEvent payload
     * @param source     Optional source pointer
     * @param target     Optional target task
     */
    Event(SystemNotification type, const GpioEvent& gpioEvent,
        size_t size = sizeof(GpioEvent),
        void* source = nullptr, FrameworkTask* target = nullptr)
      : notification(type), gpioEvent(gpioEvent), size(size), source(source), target(target) {}
  

    /**
     * @brief Construct an event with a user-defined notification.
     *
     * @param userCode User-defined notification value
     * @param data     Optional pointer to payload
     * @param size     Payload size
     * @param source   Optional source pointer
     * @param target   Optional target task
     */
    Event(uint8_t userCode, const void *data = nullptr, size_t size = 0,
          void *source = nullptr, FrameworkTask *target = nullptr)
        : notification(userCode), data(data), size(size), source(source), target(target) {}


    /// @brief Returns true if this is a user-defined event
    inline bool isUser() const {
        return notification.kind == NotificationKind::User;
    }

    /// @brief Returns true if this is a system-defined event
    inline bool isSystem() const {
        return notification.kind == NotificationKind::System;
    }

    /// @brief Returns the raw user-defined code (safe to cast in app code)
    inline uint8_t userCode() const {
        return notification.user_code;
    }

    /// @brief Returns the system-defined enum value
    inline SystemNotification systemCode() const {
        return notification.system;
    }
           
};

// global scope for convenience

/**
 * @brief Helper to create a user-defined Event with no payload.
 * 
 * @tparam Enum The enum type used for user-defined notifications.
 * @param e The enum value.
 * @return Event The constructed user event.
 */
template<typename Enum>
inline Event userEvent(Enum e) {
    return Event(static_cast<uint8_t>(e), nullptr, 0);
}

/**
 * @brief Helper to create a user-defined Event with a payload.
 * 
 * @tparam Enum The enum type.
 * @tparam T The data type of the payload.
 * @param e The enum value.
 * @param data The data to attach.
 * @return Event The constructed user event with payload.
 */
template<typename Enum, typename T>
inline Event userEvent(Enum e, const T& data) {
    return Event(static_cast<uint8_t>(e), &data, sizeof(T));
}
