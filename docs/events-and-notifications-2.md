# 📡 Events and Notifications in PicoFramework

PicoFramework supports multiple mechanisms for inter-task communication and signaling:

- 🟢 **Task Notifications** — fast, lightweight FreeRTOS signals.
- 📦 **EventManager Events** — rich, structured messages with routing and payload support.
- 🔌 **Direct GPIO Listeners** — immediate callbacks for edge-triggered GPIO events.

Understanding when and how to use each is key to writing efficient and maintainable code.

---

## 🔄 Overview

| Mechanism                | Description                                 | Transport           | Delivery Context     | Use Case                        |
|--------------------------|---------------------------------------------|---------------------|-----------------------|----------------------------------|
| **Task Notification**     | `xTaskNotifyIndexed()`                     | Task notify slot    | Immediate (RTOS-level)| Lightweight system events       |
| **EventManager Event**    | Full `Event` with routing/payload           | FreeRTOS queue      | Task context          | Structured app-level messaging  |
| **Direct GPIO Listener**  | Immediate callback via `addListener(pin)`   | Function pointer    | ISR or task context   | Low-latency GPIO response       |

---

## ⚙️ GPIO Event Handling Configuration

To control how GPIO events are handled, set the `GPIO_EVENT_HANDLING` flag in `framework_config.h`. This allows compile-time selection of the desired dispatch mode:

```cpp
#define GPIO_NOTIFICATIONS           1
#define GPIO_EVENTS                  2
#define GPIO_EVENTS_AND_NOTIFICATIONS (GPIO_NOTIFICATIONS | GPIO_EVENTS)

#define GPIO_EVENT_HANDLING GPIO_EVENTS_AND_NOTIFICATIONS
```

Then in the framework's internal handler:

```cpp
void GpioEventManager::gpio_event_handler(uint gpio, uint32_t events) {
    GpioEvent gpioEvent = {
        static_cast<uint16_t>(gpio),
        static_cast<uint16_t>(events)
    };

#if GPIO_EVENT_HANDLING & GPIO_NOTIFICATIONS
    auto it = listeners.find(gpio);
    if (it != listeners.end()) {
        for (auto& cb : it->second) {
            cb(gpioEvent);
        }
    }
#endif

#if GPIO_EVENT_HANDLING & GPIO_EVENTS
    Event evt(SystemNotification::GpioChange, gpioEvent, sizeof(GpioEvent));
    AppContext::get<EventManager>()->postEvent(evt);
#endif
}
```

---

## 🔔 Task Notifications

Lightweight FreeRTOS notifications using indexed task slots.

### ✅ Best for:
- Fast, fixed system-level signals
- No payload required

### Example

```cpp
xTaskNotifyIndexed(controller->getTaskHandle(), 0, Notification::NetworkReady.code(), eSetValueWithOverwrite);
waitFor(Notification::NetworkReady);
```

---

## 📦 EventManager Events

Structured messaging with metadata and optional payload, posted via `EventManager`.

### ✅ Best for:
- App-level events
- Broadcast or routing
- Payload data

### Example

```cpp
Event e(SystemNotification::GpioChange, gpioEvent, sizeof(GpioEvent));
EventManager::postEvent(e);
```

---

## 🔌 Direct GPIO Listeners

Register local listeners for immediate callback on GPIO change:

```cpp
gpioEventManager.addListener(pin, [](const GpioEvent& evt) {
    // Handle pin change quickly
});
```

### ✅ Best for:
- Ultra-low-latency handling
- Fast path execution
- Simple signal response

⚠️ Must be ISR-safe if invoked from interrupt context.

---

## 🧠 Summary Table

| When to Use                           | Use This Mechanism         |
|--------------------------------------|----------------------------|
| Ultra-fast, ISR-safe signal          | Direct GPIO Listener       |
| Lightweight fixed system signal      | Task Notification          |
| Structured app events or broadcast   | EventManager               |

---

## ✍️ Pro Tip

User and system notifications can share values as long as they're disambiguated via `NotificationKind`.

```cpp
enum class SystemNotification { NetworkReady = 1, ... };
enum class UserNotification { ProgramStarted = 1, ... };
```

Use the `Notification` wrapper with kind + union to route properly.
