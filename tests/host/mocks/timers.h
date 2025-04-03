#pragma once

typedef void* TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t);

#define pdPASS 1

inline TimerHandle_t xTimerCreate(const char*, int, int, void*, TimerCallbackFunction_t) { return nullptr; }
inline int xTimerStart(TimerHandle_t, int) { return pdPASS; }
inline int xTimerStop(TimerHandle_t, int) { return pdPASS; }
inline int xTimerDelete(TimerHandle_t, int) { return pdPASS; }
inline int xTimerChangePeriod(TimerHandle_t, int, int) { return pdPASS; }
inline int xTimerIsTimerActive(TimerHandle_t) { return pdPASS; }
inline int xTimerGetExpiryTime(TimerHandle_t) { return pdPASS; }
inline int xTimerGetTimerDaemonTaskHandle() { return pdPASS; }
inline int xTimerPendFunctionCall(void (*)(void*), void*, int, int) { return pdPASS; }
inline int xTimerGetTimerId(TimerHandle_t) { return pdPASS; }
inline int xTimerGetTimerName(TimerHandle_t) { return pdPASS; }
inline int xTimerGetTimerPeriod(TimerHandle_t) { return pdPASS; }
inline int xTimerGetTimerState(TimerHandle_t) { return pdPASS; }
inline int xTimerGetTimerCallbackFunction(TimerHandle_t) { return pdPASS; }
inline int xTimerGetTimerCallbackArgument(TimerHandle_t) { return pdPASS; }
inline int xTimerGetTimerQueueHandle() { return pdPASS; }
inline int xTimerGetTimerTaskHandle(TimerHandle_t) { return pdPASS; }