#pragma once

#include <functional>
#include "FreeRTOS.h"
#include "timers.h"

class TimerService {
public:
    static TimerService& instance();

    // One-shot: self-deleting
    void setTimer(uint32_t delayMs, std::function<void()> callback);

    // Repeating: continues firing until manually deleted
    TimerHandle_t setRepeatingTimer(uint32_t intervalMs, std::function<void()> callback);

private:
    TimerService() = default;
    TimerService(const TimerService&) = delete;
    TimerService& operator=(const TimerService&) = delete;

    struct TimerContext {
        TimerHandle_t handle;
        std::function<void()> callback;
        bool repeating;
    };

    static void timerCallbackStatic(TimerHandle_t xTimer);
};
