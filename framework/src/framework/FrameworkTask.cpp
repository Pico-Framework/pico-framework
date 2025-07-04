/**
 * @file FrameworkTask.cpp
 * @author Ian Archbell
 * @brief Implementation of the FrameworkTask class for FreeRTOS task abstraction.
 *
 * This source file provides the task startup, suspension, resume, notifications,
 * and optional queue support for the embedded application framework.
 *
 * @version 0.1
 * @date 2025-03-26
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "framework/FrameworkTask.h"

/// @copydoc FrameworkTask::FrameworkTask
FrameworkTask::FrameworkTask(const char *name, uint16_t stackSize, UBaseType_t priority)
    : _name(name), _stackSize(stackSize), _priority(priority) {}

/// @copydoc FrameworkTask::~FrameworkTask
FrameworkTask::~FrameworkTask()
{
    if (_handle)
    {
        vTaskDelete(_handle);
    }
    if (_queue)
    {
        vQueueDelete(_queue);
    }
}

/// @copydoc FrameworkTask::start
bool FrameworkTask::start()
{
    return xTaskCreate(taskEntry, _name, _stackSize, this, _priority, &_handle) == pdPASS;
}

/// @copydoc FrameworkTask::taskEntry
void FrameworkTask::taskEntry(void *pvParams)
{
    static_cast<FrameworkTask *>(pvParams)->run();
    vTaskDelete(nullptr); // Clean up after run ends
}

/// @copydoc FrameworkTask::suspend
void FrameworkTask::suspend()
{
    if (_handle)
        vTaskSuspend(_handle);
}

/// @copydoc FrameworkTask::resume
void FrameworkTask::resume()
{
    if (_handle)
        vTaskResume(_handle);
}

/// @copydoc FrameworkTask::getHandle
TaskHandle_t FrameworkTask::getHandle() const
{
    return _handle;
}

/// @copydoc FrameworkTask::notify
void FrameworkTask::notify(uint8_t index, uint32_t value)
{
    xTaskNotifyIndexed(_handle, index, value, eSetValueWithOverwrite);
}

/// @copydoc FrameworkTask::notify
void FrameworkTask::notify(Notification n, uint32_t value)
{
    notify(n.code(), value);
}

/// @copydoc FrameworkTask::notifyFromISR
void FrameworkTask::notifyFromISR(uint8_t index, uint32_t value, BaseType_t *pxHigherPriorityTaskWoken)
{
    xTaskNotifyIndexedFromISR(_handle, index, value, eSetValueWithOverwrite, pxHigherPriorityTaskWoken);
}

/// @copydoc FrameworkTask::notifyFromISR
void FrameworkTask::notifyFromISR(Notification n, uint32_t value, BaseType_t *pxHigherPriorityTaskWoken)
{
    notifyFromISR(n.code(), value, pxHigherPriorityTaskWoken);
}

/// @copydoc FrameworkTask::waitFor
bool FrameworkTask::waitFor(uint8_t index, TickType_t timeout)
{
    uint32_t value;
    return xTaskNotifyWaitIndexed(index, 0, UINT32_MAX, &value, timeout) == pdTRUE;
}

/// @copydoc FrameworkTask::waitFor
bool FrameworkTask::waitFor(Notification n, TickType_t timeout)
{
    return waitFor(n.code(), timeout);
}

/// @copydoc FrameworkTask::waitFor
uint32_t FrameworkTask::waitFor(TickType_t timeout)
{
    uint32_t value = 0;
    xTaskNotifyWaitIndexed(0, 0, UINT32_MAX, &value, timeout);
    return value;
}

/// @copydoc FrameworkTask::wait
Notification FrameworkTask::waitForAny(uint8_t index, uint32_t mask, TickType_t timeout)
{
    uint32_t value = 0;
    if (xTaskNotifyWaitIndexed(index, 0, mask, &value, timeout) == pdTRUE && (value & mask))
    {
        Notification n;
        n.kind = NotificationKind::System;

        // Find the *first* matching bit set — basic scan
        for (uint8_t i = 0; i < 32; ++i)
        {
            if (value & (1u << i))
            {
                n.system = static_cast<SystemNotification>(i);
                break;
            }
        }

        return n;
    }

    Notification n;
    n.kind = NotificationKind::System;
    n.system = SystemNotification::WaitForTimeout;
    return n;
}

/// @copydoc FrameworkTask::createQueue
bool FrameworkTask::createQueue(size_t itemSize, size_t length)
{
    _queue = xQueueCreate(length, itemSize);
    return _queue != nullptr;
}

/// @copydoc FrameworkTask::sendToQueue
bool FrameworkTask::sendToQueue(const void *item, TickType_t timeout)
{
    return xQueueSend(_queue, item, timeout) == pdTRUE;
}

/// @copydoc FrameworkTask::receiveFromQueue
bool FrameworkTask::receiveFromQueue(void *item, TickType_t timeout)
{
    return xQueueReceive(_queue, item, timeout) == pdTRUE;
}
