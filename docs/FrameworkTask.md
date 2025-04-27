# PicoFramework: FrameworkTask User Guide

---

## ✨ Overview

`FrameworkTask` provides a lightweight wrapper around FreeRTOS tasks.

It simplifies:
- Task creation
- Event notifications (including indexed notifications)
- Task suspension and resumption
- Optional message queue support

You subclass `FrameworkTask` to create your own FreeRTOS tasks with minimal boilerplate.

---

## 🚀 Capabilities Summary

| Capability | Description |
|:-----------|:------------|
| Task creation | Simplified startup without manually calling `xTaskCreate` |
| Indexed task notifications | Easy `notify()` and `waitFor()` methods, ISR-safe versions too |
| Suspend / Resume | Safe task control |
| Optional queue | Built-in lightweight queue support |

---

## 🔧 Creating a Framework Task

You subclass `FrameworkTask` and implement the `run()` method:

```cpp
#include "framework/FrameworkTask.h"

class MyTask : public FrameworkTask {
public:
    MyTask() : FrameworkTask("MyTask", 2048, tskIDLE_PRIORITY + 1) {}

protected:
    void run() override {
        while (true) {
            // Do some work
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
};
```

Then create and start your task:

```cpp
MyTask task;
task.start();
```

---

## 📈 Using Notifications

### Sending a notification to a task

```cpp
task.notify(0, 1); // Send notification on index 0 with value 1
```

Or send from an ISR:

```cpp
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
task.notifyFromISR(0, 1, &xHigherPriorityTaskWoken);
```

### Waiting for a notification

Inside your `run()` method:

```cpp
if (waitFor(0, pdMS_TO_TICKS(5000))) {
    // Notification received!
}
```

You can also wait using a `Notification` object if you're using event codes.

---

## 🛋️ Suspending and Resuming Tasks

```cpp
task.suspend(); // Pause the task
...
task.resume(); // Resume execution
```

---

## 🛋️ Using an Optional Queue

You can attach a lightweight queue to your task for sending structured messages.

### Creating the Queue

```cpp
task.createQueue(sizeof(MyMessage), 10); // 10 messages of size MyMessage
```

### Sending to the Queue

```cpp
MyMessage msg = { ... };
task.sendToQueue(&msg, pdMS_TO_TICKS(100));
```

### Receiving from the Queue

Inside your `run()` method:

```cpp
MyMessage msg;
if (receiveFromQueue(&msg, pdMS_TO_TICKS(500))) {
    // Process the message
}
```

---

## 👉 Full Example: Notification-based Task

```cpp
class BlinkerTask : public FrameworkTask {
public:
    BlinkerTask() : FrameworkTask("Blinker", 1024, tskIDLE_PRIORITY + 2) {}

protected:
    void run() override {
        while (true) {
            if (waitFor(0, portMAX_DELAY)) {
                // Toggle LED
                toggleLed();
            }
        }
    }

    void toggleLed() {
        // Actual hardware LED toggle code here
    }
};
```

In another part of your app:

```cpp
blinker.notify(0, 1); // Cause the LED to toggle
```

---

## 🧐 Why Use FrameworkTask?

- Standardizes FreeRTOS task creation and lifecycle
- Reduces boilerplate in your app code
- Clean notification model (event-driven)
- Optional lightweight queues without extra dependencies
- ISR-safe operations for real embedded usage

---

✅ **FrameworkTask** helps you write clean, reliable, scalable embedded tasks quickly!

