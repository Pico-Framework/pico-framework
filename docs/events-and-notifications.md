# 📡 Events vs Notifications in PicoFramework

PicoFramework supports two mechanisms for inter-task communication and signaling:

- 🟢 **Task Notifications** — fast, lightweight FreeRTOS signals.
- 📦 **EventManager Events** — rich, structured messages with routing and payload support.

Understanding when and how to use each is key to writing efficient and maintainable code.

---

## 🔄 Overview

| Feature                      | Task Notification (`waitFor`) | EventManager (`postEvent`) |
|-----------------------------|-------------------------------|----------------------------|
| Mechanism                   | `xTaskNotifyIndexed()`        | FreeRTOS Queue             |
| Payload                     | No (just an integer index)    | Yes (`Event` with optional data) |
| Routing                     | One-to-one (task only)        | Source + target routing or broadcast |
| Overhead                    | Very low                      | Higher (struct allocation and copy) |
| Use Case                    | Fast system signals           | Complex or app-defined events |
| Examples                    | `NetworkReady`, `StorageMounted` | `GpioChange`, `ConfigUpdated` |

---

## 🔔 Task Notifications

Task notifications are lightweight signals used to notify a specific task of a simple event, without sending any data. They're based on FreeRTOS's `xTaskNotifyIndexed()` function.

### ✅ Best for:
- System-level signals
- Fast, low-overhead events
- One-to-one communication

### 🧩 Code Example

```cpp
// Posting a system signal
FrameworkController* controller = AppContext::get<NetworkController>();
xTaskNotifyIndexed(controller->getTaskHandle(), 0, Notification::NetworkReady.code(), eSetValueWithOverwrite);
```

```cpp
// Waiting for the signal in a controller
void NetworkController::run()
{
    waitFor(Notification::NetworkReady);
    printf("Network is ready.
");
}
```

---

## 📦 EventManager Events

The `EventManager` provides structured delivery of richer messages, including sender, receiver, event type, and optional data. These events are routed to interested subscribers, supporting both targeted and broadcast delivery.

### ✅ Best for:
- Complex application logic
- Events with data
- Broadcast or multi-target routing

### 🧩 Code Example

```cpp
// Creating and posting a rich event
Event e;
e.source = this;
e.target = AppContext::get<ConfigController>();
e.notification = Notification::makeUser(UserNotification::ConfigUpdated);
e.data = &updatedConfig;
e.size = sizeof(updatedConfig);
EventManager::postEvent(e);
```

```cpp
// Handling the event
void ConfigController::onEvent(const Event& event)
{
    if (event.notification.kind == NotificationKind::User &&
        event.notification.user == UserNotification::ConfigUpdated)
    {
        auto* cfg = static_cast<ConfigData*>(event.data);
        applyNewConfig(*cfg);
    }
}
```

---

## 🧠 Guidelines

| When to Use                           | Use This Mechanism         |
|--------------------------------------|----------------------------|
| You need ultra-fast signaling        | `Task Notification`        |
| You are sending structured data      | `EventManager`             |
| You need multiple listeners          | `EventManager`             |
| You're signaling a single controller | Either (use case dependent)|
| You want strong separation of concerns | `EventManager`           |

---

## 💡 Summary

You can think of task notifications as **"pings"** (fast, limited, simple) and EventManager events as **"parcels"** (rich, routed, extensible).

**Both are first-class citizens** in PicoFramework. Choose the right tool based on the nature of the signal you're handling.

---

## ✍️ Pro Tip

When writing your own `Notification` enum values:

- Use `SystemNotification` for built-in system events
- Use `UserNotification` for app-defined signals
- Separate semantics are fine — values can overlap safely thanks to `NotificationKind`

```cpp
enum class SystemNotification {
    NetworkReady = 1,
    StorageMounted = 2,
    ...
};

enum class UserNotification {
    ProgramStarted = 1,
    ProgramStopped = 2,
    ...
};
```
