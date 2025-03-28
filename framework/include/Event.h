#pragma once

#include "FrameworkNotification.h"

class FrameworkTask;  // Forward declaration


struct Event {
    FrameworkNotification type = FrameworkNotification::None;  // Default to None
    FrameworkTask* target = nullptr;  // nullptr = broadcast
    void* data = nullptr;   // optional payload

    Event() = default; 

    Event(FrameworkNotification type, FrameworkTask* target, void* data = nullptr)
        : type(type), target(target), data(data) {}

    static Event broadcast(FrameworkNotification type, void* data = nullptr) {
        return Event(type, nullptr, data);
    }
};
