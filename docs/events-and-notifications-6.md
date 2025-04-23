# 📡 Events and Notifications in PicoFramework

PicoFramework provides a flexible, event-driven messaging system designed to meet the needs of real-time embedded applications. It supports two primary communication mechanisms:

- **Events**: Structured messages that flow through the `EventManager`, enabling rich pub/sub patterns and payload delivery.
- **Task Notifications**: Lightweight, low-latency signals using FreeRTOS's `xTaskNotifyIndexed`, optimized for fast point-to-point signaling.

A third mechanism—**direct GPIO listeners**—also exists as a special case for ultra-fast local GPIO event handling.

---

## 🔍 Conceptual Overview

Events and notifications serve different but complementary roles:

- **Events** are designed for flexibility, supporting targeted or broadcast delivery, optional data payloads, and decoupled communication.
- **Notifications** are intended for speed and simplicity, providing integer-only, one-to-one signaling with minimal runtime cost.

Together, these mechanisms give developers the power to choose the right level of complexity and performance for each use case.

---

## 🧭 Architecture Capabilities

### Distinction Between System and User Notifications

Notifications are implemented as a tagged union:

- `SystemNotification`: predefined framework events like `NetworkReady`, `GpioChange`, `TimeSync`
- `UserNotification`: application-defined events like `Heartbeat`, `ZoneStart`

This separation ensures:
- No collisions between framework and application domains
- Clear boundaries between internal and external responsibilities
- Easy extensibility

### Lightweight Event Structure

An `Event` consists of:
- A `Notification` (tagged union: system or user)
- Optional source (e.g., ISR or controller)
- Optional target (specific controller)
- Optional inline payload (e.g. `GpioEvent`)

It is:
- Small (typically 12–16 bytes)
- Allocation-free
- Usable in both ISR and task contexts

### Pub/Sub via EventManager

The `EventManager` acts as a central dispatcher. Events can be:
- **Broadcast** (no target specified)
- **Directed** to a specific task

Subscribers (controllers) are notified via `onEvent(const Event&)`, with filtering based on the notification type. Events are queued and delivered using FreeRTOS.

### Inline Payloads

Event payloads are passed by value using inline memory. The current implementation supports:
- `GpioEvent` (2 bytes)
- Reserve space (8 bytes) for future payload types

No heap allocations are required, which ensures deterministic memory usage.

### Type Safety

- Strong typing via `NotificationKind`
- Explicit constructors and handlers prevent misuse
- Shared `onEvent()` interface for consistent handling

---

## 📦 EventManager Usage

### Posting an Event

```cpp
Event evt;
evt.source = this;
evt.target = AppContext::get<ConfigController>();
evt.notification = Notification::makeUser(UserNotification::ConfigUpdated);
evt.data = &updatedConfig;
evt.size = sizeof(updatedConfig);
EventManager::postEvent(evt);
```

### Receiving an Event

```cpp
void ConfigController::onEvent(const Event& event) {
    if (event.notification.kind == NotificationKind::User &&
        event.notification.user == UserNotification::ConfigUpdated) {
        auto* cfg = static_cast<ConfigData*>(event.data);
        applyNewConfig(*cfg);
    }
}
```

### Best For
- Complex interactions between subsystems
- Events with payloads
- Broadcast or targeted messaging
- Decoupled logic

---

## 🔔 Task Notification Usage

### Sending a Notification

```cpp
xTaskNotifyIndexed(taskHandle, 0, Notification::NetworkReady.code(), eSetValueWithOverwrite);
```

### Receiving a Notification

```cpp
void NetworkController::run() {
    waitFor(Notification::NetworkReady);
    printf("Network is ready\n");
}
```

### Best For
- System signals that require fast response
- Simple one-to-one communication
- Notification without data payload

---

## ⚙️ GPIO Event Handling

The GPIO system bridges both models:

- **Direct listeners** for fast ISR-based callbacks
- **EventManager posting** for structured handling

Configure via `framework_config.h`:

```cpp
#define GPIO_NOTIFICATIONS           1
#define GPIO_EVENTS                  2
#define GPIO_EVENTS_AND_NOTIFICATIONS (GPIO_NOTIFICATIONS | GPIO_EVENTS)
#define GPIO_EVENT_HANDLING GPIO_EVENTS_AND_NOTIFICATIONS
```

Example handler:

```cpp
void GpioEventManager::gpio_event_handler(uint gpio, uint32_t events) {
    GpioEvent gpioEvent = { static_cast<uint16_t>(gpio), static_cast<uint16_t>(events) };

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

### Best For
- Low-latency GPIO response
- Bridging real-time GPIO events to framework-level pub/sub
- Optional use of callbacks, events, or both

---

## 📊 Feature Comparison

| Feature                                 | Task Notifications | EventManager Events | GPIO Listeners |
|----------------------------------------|--------------------|---------------------|----------------|
| Payload support                         | ❌                 | ✅                  | ✅              |
| One-to-one delivery                     | ✅                 | ✅                  | ✅              |
| Broadcast                               | ❌                 | ✅                  | ❌              |
| ISR-safe                                | ✅                 | ⚠️ Depends on queue | ✅              |
| Low-latency                             | ✅                 | ❌                  | ✅              |
| Type safety                             | Moderate           | High                | Moderate        |
| Structured/event-driven                 | ❌                 | ✅                  | ❌              |
| Best for critical system signals        | ✅                 | ❌                  | ✅              |
| Best for structured app communication   | ❌                 | ✅                  | ❌              |

