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


// Usage example for notifyFromISR : higherPriorityTaskWoken is set to pdTRUE if a higher priority task was woken by the notification.
// BaseType_t higherPriorityTaskWoken = pdFALSE;
// app->notifyFromISR(SystemNotification::NetworkReady, 1, &higherPriorityTaskWoken);
// portYIELD_FROM_ISR(higherPriorityTaskWoken);

#include "FrameworkTask.h"

void FrameworkTask::notify(uint8_t index, uint32_t value) {
    xTaskNotifyIndexed(_handle, index, value, eSetValueWithOverwrite);
}

void FrameworkTask::notify(FrameworkNotification n, uint32_t value) {
    notify(static_cast<uint8_t>(n), value);
}

void FrameworkTask::notifyFromISR(uint8_t index, uint32_t value, BaseType_t* pxHigherPriorityTaskWoken) {
    xTaskNotifyIndexedFromISR(_handle, index, value, eSetValueWithOverwrite, pxHigherPriorityTaskWoken);
}

void FrameworkTask::notifyFromISR(FrameworkNotification n, uint32_t value, BaseType_t* pxHigherPriorityTaskWoken) {
    notifyFromISR(static_cast<uint8_t>(n), value, pxHigherPriorityTaskWoken);
}

bool FrameworkTask::waitFor(uint8_t index, TickType_t timeout) {
    uint32_t value;
    return xTaskNotifyWaitIndexed(index, 0, UINT32_MAX, &value, timeout) == pdTRUE;
}

bool FrameworkTask::waitFor(FrameworkNotification n, TickType_t timeout) {
    return waitFor(static_cast<uint8_t>(n), timeout);
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
