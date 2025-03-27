#include "FrameworkController.h"

FrameworkController::FrameworkController(const char* name, uint16_t stackSize, UBaseType_t priority)
    : FrameworkTask(name, stackSize, priority) {}

void FrameworkController::run() {
    onStart();
    while (true) {
        waitAndDispatch(100);  // Wait for notifications or timeout
        poll();                // Call user logic
    }
}

void FrameworkController::onStart() {
    // Default no-op
}

void FrameworkController::onEvent(uint32_t eventMask) {
    // Default no-op
}

void FrameworkController::poll() {
    // Default no-op
}

void FrameworkController::waitAndDispatch(uint32_t timeoutMs) {
    uint32_t eventMask = waitForNotification(pdMS_TO_TICKS(timeoutMs));
    if (eventMask != 0) {
        onEvent(eventMask);
    }
}

void FrameworkController::runEvery(uint32_t intervalMs, const std::function<void()>& fn, const char* id) {
    TickType_t now = xTaskGetTickCount();
    TickType_t& last = _timers[std::string(id)];

    if ((now - last) >= pdMS_TO_TICKS(intervalMs)) {
        fn();
        last = now;
    }
}
