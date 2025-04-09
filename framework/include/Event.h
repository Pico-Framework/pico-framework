/**
 * @file Event.h
 * @brief Defines the Event structure and related utilities for event messaging.
 * @version 1.0
 * @date 2025-04-09
 * @copyright Copyright (c) 2025, Ian Archbell
 * @license MIT
 */

#pragma once
#include <cstdint>
#include "FrameworkNotification.h" // Must be included before alias
class FrameworkTask;
/**
 * @brief Alias for the type used to identify events.
 */
using EventType = FrameworkNotification;

/**
 * @brief Represents a framework event, optionally carrying payload data.
 */
struct Event
{
    EventType type = EventType::None; ///< Event type identifier
    const void *data = nullptr;       ///< Pointer to event payload data
    size_t size = 0;                  ///< Size of payload data
    void *source = nullptr;           ///< Optional source (e.g. task)
    FrameworkTask *target = nullptr;  ///< Optional specific target (for directed delivery)

    enum class FrameworkNotification : uint8_t
    {
        // Reserved for system/framework use
        None = 0,
        NetworkReady = 1,
        TimeSync = 2,
        ConfigUpdated = 3,
        OTAAvailable = 4,
        GpioChange = 5,

        // Users are free to add their own from here
        UserBase = 16,

        // Example user-defined notifications
        ControllerReady = UserBase,
        ProgramEnd = 17,
        RunProgram = 18,
        ZoneEndTime = 19,
        ProgramStartTime = 20,
        TimerTick = 21,
    };

    /**
     * @brief Default constructor (creates a None event).
     */
    Event() = default;

    /**
     * @brief Construct a new Event with optional payload.
     *
     * @param type   Event type (e.g. EventType::RunProgram)
     * @param data   Pointer to optional payload
     * @param size   Size of payload
     * @param source Optional source (e.g. task pointer)
     */
    Event(EventType type, const void* data = nullptr, size_t size = 0, void* source = nullptr, FrameworkTask* target = nullptr)
    : type(type), data(data), size(size), source(source), target(target) {}
};
