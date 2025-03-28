/**
 * @file EventManager.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "EventManager.h"
#include "FrameworkTask.h" // Include your Task class header
#include "pico/stdlib.h" 
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "utility.h" // Include utility functions for is_in_interrupt()

EventManager::EventManager(size_t queueSize) {
    eventQueue_ = xQueueCreate(queueSize, sizeof(Event));
}

void EventManager::subscribe(uint32_t eventMask, FrameworkTask* task) {
    subscribers_.push_back({ eventMask, task });
}

void EventManager::postEvent(const Event& event) {
    BaseType_t xHigherPriTaskWoken = pdFALSE;

    if (is_in_interrupt()) {
        xQueueSendToBackFromISR(eventQueue_, &event, &xHigherPriTaskWoken);

        for (auto& sub : subscribers_) {
            if (sub.eventMask & (1u << static_cast<uint8_t>(event.type))) {
                sub.task->notifyFromISR(static_cast<uint8_t>(event.type), 1, &xHigherPriTaskWoken);
            }
        }

        portYIELD_FROM_ISR(xHigherPriTaskWoken);
    } else {
        xQueueSendToBack(eventQueue_, &event, 0);

        for (auto& sub : subscribers_) {
            if (sub.eventMask & (1u << static_cast<uint8_t>(event.type))) {
                sub.task->notify(static_cast<uint8_t>(event.type));
            }
        }
    }
}


bool EventManager::getNextEvent(Event& event, uint32_t timeoutMs) const {
    return xQueueReceive(eventQueue_, &event, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

bool EventManager::hasPendingEvents() const {
    return uxQueueMessagesWaiting(eventQueue_) > 0;
}
