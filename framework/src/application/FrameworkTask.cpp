/**
 * @file FreeRTOSTask.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "FrameworkTask.h"

 FrameworkTask::FrameworkTask(const char* name, uint16_t stackSize, UBaseType_t priority)
    : _name(name), _stackSize(stackSize), _priority(priority) {}

FrameworkTask::~FrameworkTask() {
    if (_handle) {
        vTaskDelete(_handle);
    }
    if (_queue) {
        vQueueDelete(_queue);
    }
}

bool FrameworkTask::start() {
    return xTaskCreate(taskEntry, _name, _stackSize, this, _priority, &_handle) == pdPASS;
}

void FrameworkTask::taskEntry(void* pvParams) {
    static_cast<FrameworkTask*>(pvParams)->run();
    vTaskDelete(nullptr); // Clean up after run ends
}

void FrameworkTask::suspend() {
    if (_handle) vTaskSuspend(_handle);
}

void FrameworkTask::resume() {
    if (_handle) vTaskResume(_handle);
}

TaskHandle_t FrameworkTask::getHandle() const {
    return _handle;
}

void FrameworkTask::notify(SystemNotification type, uint32_t value) {
    xTaskNotifyIndexed(
        _handle,
        static_cast<uint8_t>(type),
        value,
        eSetValueWithOverwrite
    );
}

// Usage example for notifyFromISR : higherPriorityTaskWoken is set to pdTRUE if a higher priority task was woken by the notification.
// BaseType_t higherPriorityTaskWoken = pdFALSE;
// app->notifyFromISR(SystemNotification::NetworkReady, 1, &higherPriorityTaskWoken);
// portYIELD_FROM_ISR(higherPriorityTaskWoken);

void FrameworkTask::notifyFromISR(SystemNotification type, uint32_t value, BaseType_t* higherPriorityTaskWoken) {
    xTaskNotifyIndexedFromISR(
        _handle,
        static_cast<uint8_t>(type),
        value,
        eSetValueWithOverwrite,
        higherPriorityTaskWoken
    );
}

bool FrameworkTask::waitFor(SystemNotification type, TickType_t timeout) {
    uint32_t value;
    return xTaskNotifyWaitIndexed(
        static_cast<uint8_t>(type),
        0,
        UINT32_MAX,
        &value,
        timeout
    ) == pdTRUE;
}

// --- Notifications ---
void FrameworkTask::notify(uint32_t value) {
    xTaskNotify(_handle, value, eSetValueWithOverwrite);
}

// --- Notifications from ISR ---
void FrameworkTask::notifyFromISR(uint32_t value, BaseType_t* pxHigherPriorityTaskWoken) {
    xTaskNotifyFromISR(_handle, value, eSetValueWithOverwrite, pxHigherPriorityTaskWoken);
}

uint32_t FrameworkTask::waitForNotification(TickType_t timeout) {
    uint32_t value = 0;
    xTaskNotifyWait(0, 0, &value, timeout);
    return value;
}

// --- Queue Support ---
bool FrameworkTask::createQueue(size_t itemSize, size_t length) {
    _queue = xQueueCreate(length, itemSize);
    return _queue != nullptr;
}

bool FrameworkTask::sendToQueue(const void* item, TickType_t timeout) {
    return xQueueSend(_queue, item, timeout) == pdTRUE;
}

bool FrameworkTask::receiveFromQueue(void* item, TickType_t timeout) {
    return xQueueReceive(_queue, item, timeout) == pdTRUE;
}
