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

 #include "EventManager.h"
 #include "FrameworkTask.h"
 #include "pico/stdlib.h"
 #include "FreeRTOS.h"
 #include "task.h"
 #include "portmacro.h"
 #include "utility.h" // for is_in_interrupt()
 
 /// @copydoc EventManager::EventManager
 EventManager::EventManager(size_t queueSize) {
     eventQueue_ = xQueueCreate(queueSize, sizeof(Event));
 }
 
 /// @copydoc EventManager::subscribe
 void EventManager::subscribe(uint32_t eventMask, FrameworkTask* task) {
     subscribers_.push_back({ eventMask, task });
 }
 
 /// @copydoc EventManager::postEvent
 void EventManager::postEvent(const Event& event) {
     BaseType_t xHigherPriTaskWoken = pdFALSE;
 
     if (is_in_interrupt()) {
         xQueueSendToBackFromISR(eventQueue_, &event, &xHigherPriTaskWoken);
 
         for (auto& sub : subscribers_) {
             if ((sub.eventMask & (1u << static_cast<uint8_t>(event.type))) &&
                 (event.target == nullptr || sub.task == event.target)) {
                 sub.task->notifyFromISR(static_cast<uint8_t>(event.type), 1, &xHigherPriTaskWoken);
             }
         }
 
         portYIELD_FROM_ISR(xHigherPriTaskWoken);
     } else {
         xQueueSendToBack(eventQueue_, &event, 0);
 
         for (auto& sub : subscribers_) {
             if ((sub.eventMask & (1u << static_cast<uint8_t>(event.type))) &&
                 (event.target == nullptr || sub.task == event.target)) {
                 sub.task->onEvent(event);
             }
         }
     }
 }
 
 /// @copydoc EventManager::getNextEvent
 bool EventManager::getNextEvent(Event& event, uint32_t timeoutMs) const {
     return xQueueReceive(eventQueue_, &event, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
 }
 
 /// @copydoc EventManager::hasPendingEvents
 bool EventManager::hasPendingEvents() const {
     return uxQueueMessagesWaiting(eventQueue_) > 0;
 }
 