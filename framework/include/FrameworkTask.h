/**
 * @file FreeRTOSTask.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef FREERTOS_TASK_H
#define FREERTOS_TASK_H
#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "FrameworkNotification.h"
class FrameworkTask {

    public:
    FrameworkTask(const char* name, uint16_t stackSize = 1024, UBaseType_t priority = 1);
    virtual ~FrameworkTask();

    bool start();
    void suspend();
    void resume();
    TaskHandle_t getHandle() const;

    // Notify
    void notify(uint8_t index, uint32_t value = 1);
    void notify(FrameworkNotification n, uint32_t value = 1);

    // Notify from ISR
    void notifyFromISR(uint8_t index, uint32_t value = 1, BaseType_t* pxHigherPriorityTaskWoken = nullptr);
    void notifyFromISR(FrameworkNotification n, uint32_t value = 1, BaseType_t* pxHigherPriorityTaskWoken = nullptr);

    // Wait
    bool waitFor(uint8_t index, TickType_t timeout = portMAX_DELAY);
    bool waitFor(FrameworkNotification n, TickType_t timeout = portMAX_DELAY);

protected:
    virtual void run() = 0; // override in subclass

    uint32_t waitForNotification(TickType_t timeout = portMAX_DELAY);

    // For optional message queue
    bool createQueue(size_t itemSize, size_t length);
    bool sendToQueue(const void* item, TickType_t timeout = 0);
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


// Integration with EventManager and EventListener

// class MyTask : public Task, public EventListener {
//     public:
//         MyTask() : Task("MyTask", 1024, 1) {}
    
//     protected:
//         void run() override {
//             while (true) {
//                 Event evt = waitForEvent();  // hypothetically wraps queue or notification
//                 handleEvent(evt);
//             }
//         }
    
//         void onEvent(const Event& e) override {
//             sendToQueue(&e, 0);  // Or notify depending on setup
//         }
//     };
    

//#pragma once

// class Event;

// class EventListener {
// public:
//     virtual ~EventListener() = default;

//     // Called when an event is dispatched to this listener
//     virtual void onEvent(const Event& event) = 0;
// };


// #pragma once
// #include <cstdint>

// class Event {
// public:
//     uint32_t type;
//     void* data;

//     Event(uint32_t type = 0, void* data = nullptr)
//         : type(type), data(data) {}
// };


// class MyTask : public Task, public EventListener {
//     public:
//         MyTask() : Task("MyTask", 1024, 1) {}
    
//     protected:
//         void run() override {
//             // task main loop
//         }
    
//         void onEvent(const Event& event) override {
//             // handle event, e.g., push to queue
//         }
//     };
    