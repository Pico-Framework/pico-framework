#pragma once
// tests/mocks/FreeRTOS.h

#include <cstddef>
#include <cstdint>
#include <stdio.h>

using UBaseType_t = unsigned int;

// Minimal FreeRTOS task stubs for testing
typedef void (*TaskFunction_t)(void*);
typedef void* TaskHandle_t;

inline void vTaskDelay(int) {}
inline void vTaskDelete(TaskHandle_t) {}
inline void vTaskSuspend(TaskHandle_t) {}
inline void xTaskCreatePinnedToCore(...) {} // if needed

#define pdMS_TO_TICKS(ms) (ms)
typedef char StackType_t;
typedef struct { int dummy; } StaticTask_t;

struct TaskStatus_t {
    const char* pcTaskName;
    UBaseType_t xTaskNumber;
    UBaseType_t eCurrentState;
    UBaseType_t uxCurrentPriority;
    UBaseType_t uxBasePriority;
    size_t usStackHighWaterMark;
};

inline void vTaskGetInfo(void*, TaskStatus_t* pxTaskStatus, int, int) {
    if (pxTaskStatus) {
        *pxTaskStatus = {"MockTask", 3, 2, 2, 128};
    }
}

struct HeapStats_t {
    size_t totalHeapSize;
    size_t freeHeapSize;
    size_t minimumEverFreeHeapSize;
    size_t xAvailableHeapSpaceInBytes;
    size_t xNumberOfFreeBlocks;
    size_t xNumberOfSuccessfulAllocations;
    size_t xNumberOfSuccessfulFrees;
};

// Constants & macros
#define pdTRUE 1
#define pdFALSE 0
#define eInvalid -1
#define configASSERT(x) ((void)0)

inline TaskStatus_t* pxTaskStatusArray = nullptr;

inline void* pvPortMalloc(size_t) { return nullptr; }
inline void vPortFree(void*) {}

inline size_t xPortGetFreeHeapSize() { return 10000; }
inline size_t xPortGetMinimumEverFreeHeapSize() { return 5000; }
inline size_t uxTaskGetStackHighWaterMark(void*) { return 128; }
inline UBaseType_t uxTaskGetNumberOfTasks() { return 2; }

inline UBaseType_t uxTaskGetSystemState(TaskStatus_t* array, UBaseType_t count, unsigned long* totalRunTime){
    if (array) {
        array[0] = { "MockTask", 1, 1, 100 };
    }
    if (totalRunTime) *totalRunTime = 1000;
    return 1;
}

inline void* xTaskGetHandle(const char*) { return nullptr; }

inline void panic(const char* msg) {
    printf("PANIC: %s\n", msg);
}

inline void vPortGetHeapStats(HeapStats_t* stats) {
    stats->totalHeapSize = 10000;
    stats->freeHeapSize = 4000;
    stats->minimumEverFreeHeapSize = 3000;
    stats->xAvailableHeapSpaceInBytes = 4000;
    stats->xNumberOfFreeBlocks = 10;
    stats->xNumberOfSuccessfulAllocations = 500;
    stats->xNumberOfSuccessfulFrees = 480;
}