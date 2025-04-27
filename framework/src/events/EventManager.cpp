/**
 * @file EventManager.h
 * @author Ian Archbell
 * @brief Event pub/sub manager for embedded applications using FreeRTOS.
 *
 * Part of the PicoFramework application framework.
 * This module provides the EventManager class, which allows tasks to subscribe
 * to specific event types and post events to a global queue.
 *
 * The EventManager uses a FreeRTOS queue to manage events and supports
 * both task and ISR contexts for posting events.
 *
 * Tasks can subscribe to specific event types using a bitmask and receive notifications
 * via `onEvent()` or FreeRTOS task notifications. It is possible to broadcast events
 * to all subscribers or target specific tasks based on their subscriptions.
 *
 * This design allows for flexible event-driven programming in embedded systems,
 * enabling modular and decoupled task management.
 *
 * The EventManager is implemented as a singleton, ensuring that there is
 * only one global event queue shared across the application.
 *
 * @note Events are represented by the `Event` class, which encapsulates the event type
 * and any associated data. You can extend the `Event` class to include additional
 * information as needed for your application.
 *
 * @note This is designed to be used in conjunction with the FrameworkTask class,
 * which provides the `onEvent()` method for handling events.
 *
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "events/EventManager.h"
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <portmacro.h>
#include "framework/FrameworkTask.h"
#include "utility/utility.h" // for is_in_interrupt()
#include "framework/FrameworkController.h"

/// @copydoc EventManager::EventManager
EventManager::EventManager(size_t queueSize)
{
    lock = xSemaphoreCreateMutex();
    configASSERT(lock);
}

/// @copydoc EventManager::subscribe
void EventManager::subscribe(uint32_t mask, FrameworkController* target)
{
    withSubscribers([&](auto& subs) {
        subs.push_back({mask, target});
    });
}

void EventManager::withSubscribers(const std::function<void(std::vector<Subscriber>&)>& fn)
{
    xSemaphoreTake(lock, portMAX_DELAY);
    fn(subscribers_);
    xSemaphoreGive(lock);
}

/// @copydoc EventManager::postEvent
void EventManager::postEvent(const Event& event)
{
    const uint8_t code = event.notification.code();

    if (is_in_interrupt()) {
        BaseType_t xHigherPriTaskWoken = pdFALSE;

        withSubscribers([&](auto& subs) {
            for (auto& sub : subs) {
                if ((sub.eventMask & (1u << code)) &&
                    (event.target == nullptr || sub.controller == event.target))
                {
                    QueueHandle_t q = sub.controller->getEventQueue();
                    if (q) {
                        BaseType_t result = xQueueSendToBackFromISR(q, &event, &xHigherPriTaskWoken);
                        if (result != pdPASS) {
                            debug_print("[EventManager] xQueueSendFromISR FAILED — queue full!\n");
                        } else {
                            sub.controller->notifyFromISR(code, 1, &xHigherPriTaskWoken);
                        }
                    }
                }
            }
        });

        portYIELD_FROM_ISR(xHigherPriTaskWoken);
    } else {
        withSubscribers([&](auto& subs) {
            for (auto& sub : subs) {
                if ((sub.eventMask & (1u << code)) &&
                    (event.target == nullptr || sub.controller == event.target))
                {
                    QueueHandle_t q = sub.controller->getEventQueue();
                    if (q) {
                        if (xQueueSendToBack(q, &event, 0) != pdPASS) {
                            debug_print("[EventManager] xQueueSend FAILED — queue full!\n");
                        }
                    }
                }
            }
        });
    }
}
