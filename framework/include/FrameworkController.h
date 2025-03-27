#pragma once

#include "FrameworkTask.h"
#include <unordered_map>
#include <functional>
#include "FreeRTOS.h"
#include "task.h"
#include <string>

class FrameworkController : public FrameworkTask {
public:
    FrameworkController(const char* name, uint16_t stackSize = 1024, UBaseType_t priority = tskIDLE_PRIORITY + 1);
    void run() override final;

protected:
    virtual void onStart();                      // Called once at task start
    virtual void onEvent(uint32_t eventMask);    // Called when notified
    virtual void poll();                         // Non-blocking periodic work

    // Polling helper
    void runEvery(uint32_t intervalMs, const std::function<void()>& fn, const char* id);

private:
    std::unordered_map<std::string, TickType_t> _timers;

    void waitAndDispatch(uint32_t timeoutMs = portMAX_DELAY);
};

// Macro to auto-generate unique ID from line number
#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)
#define RUN_EVERY(ms, fn) runEvery(ms, fn, __FUNCTION__ ":" STRINGIFY(__LINE__))
