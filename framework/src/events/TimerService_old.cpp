#include "TimerService.h"
#include <cstdio>

TimerService& TimerService::instance() {
    static TimerService singleton;
    return singleton;
}

void TimerService::setTimer(uint32_t delayMs, std::function<void()> callback) {
    auto* ctx = new TimerContext{ nullptr, std::move(callback), false };

    ctx->handle = xTimerCreate(
        "OneShotTimer",
        pdMS_TO_TICKS(delayMs),
        pdFALSE,  // one-shot
        ctx,
        timerCallbackStatic
    );

    if (ctx->handle && xTimerStart(ctx->handle, 0) == pdPASS) {
        return;
    }

    printf("TimerService: Failed to start one-shot timer\n");
    if (ctx->handle) xTimerDelete(ctx->handle, 0);
    delete ctx;
}

TimerHandle_t TimerService::setRepeatingTimer(uint32_t intervalMs, std::function<void()> callback) {
    auto* ctx = new TimerContext{ nullptr, std::move(callback), true };

    ctx->handle = xTimerCreate(
        "RepeatingTimer",
        pdMS_TO_TICKS(intervalMs),
        pdTRUE,  // repeating
        ctx,
        timerCallbackStatic
    );

    if (ctx->handle && xTimerStart(ctx->handle, 0) == pdPASS) {
        return ctx->handle;
    }

    printf("TimerService: Failed to start repeating timer\n");
    if (ctx->handle) xTimerDelete(ctx->handle, 0);
    delete ctx;
    return nullptr;
}

void TimerService::timerCallbackStatic(TimerHandle_t xTimer) {
    auto* ctx = static_cast<TimerContext*>(pvTimerGetTimerID(xTimer));
    if (ctx && ctx->callback) {
        ctx->callback();
    }

    if (!ctx || !ctx->repeating) {
        xTimerDelete(xTimer, 0);
        delete ctx;
    }
}
