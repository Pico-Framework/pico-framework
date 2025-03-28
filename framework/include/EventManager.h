/**
 * @file EventManager.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H
#pragma once

#include <vector>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "Event.h"

// Forward declaration of your Task wrapper class
class FrameworkTask;

enum class EventType : uint8_t {
    None = 0,
    SysStartup,
    SysError,
    UserInput,
    UserCommand,
    // Extend as needed
};

class EventManager {
public:
    // Singleton pattern to ensure only one instance of EventManager
    static EventManager& getInstance() {
        static EventManager instance;
        return instance;
    }

    explicit EventManager(size_t queueSize = 32);

    // Subscribe a task to specific events (eventMask is a bitmask of 1 << EventType)
    void subscribe(uint32_t eventMask, FrameworkTask* task);

    // Post an event to the queue and notify all relevant subscribers
    void postEvent(const Event& event);

    // Called by tasks to retrieve the next event (non-blocking if timeout = 0)
    bool getNextEvent(Event& event, uint32_t timeoutMs = portMAX_DELAY) const;

    // Check if any events are currently pending
    bool hasPendingEvents() const;

private:
    struct Subscriber {
        uint32_t eventMask;
        FrameworkTask* task;
    };

    std::vector<Subscriber> subscribers_;
    QueueHandle_t eventQueue_;
};

#endif // EVENT_MANAGER_H