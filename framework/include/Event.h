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
#include "Notification.h"

class FrameworkTask;

/**
 * @brief Represents a framework event, optionally carrying payload data.
 */
struct Event
{
    Notification notification;       ///< Notification identifier (system or user)
    const void *data = nullptr;      ///< Pointer to event payload data
    size_t size = 0;                 ///< Size of payload data
    void *source = nullptr;          ///< Optional source (e.g. task)
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
};
