/**
 * @file FrameworkTask.h
 * @author Ian Archbell
 * @brief FreeRTOS-aware task abstraction with built-in notification and queue support.
 * 
 * This class provides a clean interface for managing FreeRTOS tasks, handling notifications,
 * and optionally receiving events or messages via a queue. Tasks subclass this base and
 * override the `run()` method to define their main loop.
 * 
 * Integrates cleanly with the EventManager and notification system via the `onEvent()` callback.
 * 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #ifndef FREERTOS_TASK_H
 #define FREERTOS_TASK_H
 #pragma once
 
 #include "FreeRTOS.h"
 #include "task.h"
 #include "queue.h"
 #include "Event.h"
 #include "FrameworkNotification.h"
 
 /**
  * @brief Base class for FreeRTOS-aware tasks in the framework.
  * 
  * Subclass this to define application or system-level tasks. 
  * - Override `run()` to define the main task loop.
  * - Optionally override `onEvent()` to receive posted events.
  * 
  * Supports:
  * - Indexed task notifications (including from ISR)
  * - Optional message queue for passing data
  * - Integration with the framework event system
  */
 class FrameworkTask {
 public:
     /**
      * @brief Constructor.
      * 
      * @param name Task name (used by FreeRTOS).
      * @param stackSize Stack size in words.
      * @param priority Task priority.
      */
     FrameworkTask(const char* name, uint16_t stackSize = 1024, UBaseType_t priority = 1);
 
     /**
      * @brief Destructor. Frees the task and its queue (if allocated).
      */
     virtual ~FrameworkTask();
 
     /**
      * @brief Called when the task receives an event.
      * 
      * Override to react to framework events. This is called by the event manager.
      * Default implementation does nothing.
      */
     virtual void onEvent(const Event& event) {}
 
     /**
      * @brief Starts the task via FreeRTOS.
      * 
      * Internally calls `xTaskCreate()`. The task runs `run()` when scheduled.
      * 
      * @return true if task was successfully created.
      */
     bool start();
 
     /**
      * @brief Suspends the task using `vTaskSuspend()`.
      */
     void suspend();
 
     /**
      * @brief Resumes the task using `vTaskResume()`.
      */
     void resume();
 
     /**
      * @brief Returns the FreeRTOS task handle.
      */
     TaskHandle_t getHandle() const;
 
     // ----------- Notification Support -----------
 
     /**
      * @brief Sends a notification to this task using an index.
      * 
      * @param index Notification slot index (0–7).
      * @param value Value to send (default is 1).
      */
     void notify(uint8_t index, uint32_t value = 1);
 
     /**
      * @brief Sends a notification using a framework-defined enum.
      */
     void notify(FrameworkNotification n, uint32_t value = 1);
 
     /**
      * @brief Sends a notification from an ISR (by index).
      */
     void notifyFromISR(uint8_t index, uint32_t value = 1, BaseType_t* pxHigherPriorityTaskWoken = nullptr);
 
     /**
      * @brief Sends a notification from ISR using enum identifier.
      */
     void notifyFromISR(FrameworkNotification n, uint32_t value = 1, BaseType_t* pxHigherPriorityTaskWoken = nullptr);
 
     /**
      * @brief Waits for a notification (by index).
      * 
      * @param index Notification index to wait for.
      * @param timeout Timeout in ticks.
      * @return true if notified, false on timeout.
      */
     bool waitFor(uint8_t index, TickType_t timeout = portMAX_DELAY);
 
     /**
      * @brief Waits for a notification (by enum identifier).
      */
     bool waitFor(FrameworkNotification n, TickType_t timeout = portMAX_DELAY);
 
 protected:
     /**
      * @brief Main task loop. Must be implemented by subclasses.
      */
     virtual void run() = 0;
 
     /**
      * @brief Wait for any notification (default index).
      */
     uint32_t waitFor(TickType_t timeout = portMAX_DELAY);
 
     // ----------- Optional Queue Support -----------
 
     /**
      * @brief Creates an internal FreeRTOS queue.
      * 
      * @param itemSize Size of each message item.
      * @param length Number of slots in the queue.
      * @return true if queue was successfully created.
      */
     bool createQueue(size_t itemSize, size_t length);
 
     /**
      * @brief Sends an item to the internal queue.
      */
     bool sendToQueue(const void* item, TickType_t timeout = 0);
 
     /**
      * @brief Receives an item from the internal queue.
      */
     bool receiveFromQueue(void* item, TickType_t timeout = portMAX_DELAY);
 
 protected:
     const char* _name;
     uint16_t _stackSize;
     UBaseType_t _priority;
     TaskHandle_t _handle = nullptr;
     QueueHandle_t _queue = nullptr;
 
 private:
     static void taskEntry(void* pvParams);
 };
 
 #endif // FREERTOS_TASK_H
 