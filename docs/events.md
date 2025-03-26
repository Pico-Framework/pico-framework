Each task can subscribe to one or more events using a bitmask. Example:

eventManager.subscribe((1 << static_cast<uint8_t>(EventType::SysStartup)) |
                       (1 << static_cast<uint8_t>(EventType::UserInput)),
                       xTaskGetCurrentTaskHandle());

Tasks use ulTaskNotifyTake(pdTRUE, 0) to non-blockingly check for notifications, then getNextEvent() to retrieve the data.
ISR safety is included using __get_IPSR() (CMSIS) to detect interrupt context.



#include "event_manager.h"

// Global instance of the event manager
EventManager eventManager;

void UserEventTask(void* params) {
    // Subscribe to UserInput and UserCommand events
    eventManager.subscribe(
        (1 << static_cast<uint8_t>(EventType::UserInput)) |
        (1 << static_cast<uint8_t>(EventType::UserCommand)),
        xTaskGetCurrentTaskHandle()
    );

    Creating a task that subscribes to events

    Event evt;

    while (true) {
        if (ulTaskNotifyTake(pdTRUE, 0) > 0) {  // Non-blocking notification check
            while (eventManager.getNextEvent(evt, 0)) {
                switch (evt.type) {
                    case EventType::UserInput:
                        // handle user input
                        printf("User input event received\n");
                        break;
                    case EventType::UserCommand:
                        // handle command
                        printf("User command event received\n");
                        break;
                    default:
                        break;
                }
            }
        }

        // Task continues doing other things...
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


Creating a system event task

void SystemEventTask(void* params) {
    eventManager.subscribe(
        (1 << static_cast<uint8_t>(EventType::SysStartup)) |
        (1 << static_cast<uint8_t>(EventType::SysError)),
        xTaskGetCurrentTaskHandle()
    );

    Event evt;

    while (true) {
        if (ulTaskNotifyTake(pdTRUE, 0) > 0) {
            while (eventManager.getNextEvent(evt, 0)) {
                if (evt.type == EventType::SysError) {
                    printf("System error occurred!\n");
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

Posting events

void simulateEvents() {
    Event userEvt = { EventType::UserInput, nullptr };
    eventManager.postEvent(userEvt);

    Event sysEvt = { EventType::SysError, nullptr };
    eventManager.postEvent(sysEvt);
}


From ISR

void GPIO_IRQHandler() {
    Event evt = { EventType::UserInput, nullptr };
    eventManager.postEvent(evt);  // Safe in ISR context
}

Revised using Task wrapper
Let EventManager store pointers to your task wrappers instead of raw FreeRTOS task handles:

class EventManager {
public:
    void subscribe(uint32_t eventMask, Task* task);

    void postEvent(const Event& event);
    bool getNextEvent(Event& event, uint32_t timeoutMs = portMAX_DELAY) const;
    bool hasPendingEvents() const;

private:
    struct Subscriber {
        uint32_t eventMask;
        Task* task;
    };

    std::vector<Subscriber> subscribers_;
    QueueHandle_t eventQueue_;
};


Inside postEvent()

for (auto& sub : subscribers_) {
    if (sub.eventMask & (1u << static_cast<uint8_t>(event.type))) {
        sub.task->notify();  // Use your wrapper instead of xTaskNotifyGive
    }
}


class UserEventTask : public Task {
public:
    void onStart() {
        eventManager.subscribe(
            (1 << static_cast<uint8_t>(EventType::UserInput)) |
            (1 << static_cast<uint8_t>(EventType::UserCommand)),
            this
        );
    }

    void run() override {
        Event evt;

        while (true) {
            if (hasNotification()) {
                while (eventManager.getNextEvent(evt, 0)) {
                    handleEvent(evt);
                }
            }

            // Do other work
            delay(50);  // maybe wraps vTaskDelay
        }
    }

private:
    void handleEvent(const Event& evt) {
        // Custom user logic
    }
};

