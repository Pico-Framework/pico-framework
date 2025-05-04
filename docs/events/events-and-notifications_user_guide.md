
# Events and Notifications in PicoFramework

PicoFramework provides multiple mechanisms for inter-task communication and asynchronous signaling. These mechanisms are designed to meet the needs of embedded systems developers who require a mix of speed, flexibility, and reliability.

There are three core signaling mechanisms available in the framework:

- Task Notifications: These are fast, lightweight signals delivered directly to tasks using FreeRTOS primitives.
- EventManager Events: These are structured messages that support routing, payloads, and pub-sub communication between components.
- GPIO Listeners: These are low-latency callbacks used for fast response to GPIO changes, especially in interrupt service routines.

Choosing the right mechanism for a given use case depends on performance requirements, data complexity, and system architecture.

---

## Overview and Feature Comparison

The following table summarizes the key differences between the three communication mechanisms:

| Feature                                 | Task Notifications | EventManager Events | GPIO Listeners     |
|----------------------------------------|--------------------|---------------------|--------------------|
| Payload support                         | ✗                  | ✓                   | ✓                  |
| One-to-one delivery                     | ✓                  | ✓                   | ✓                  |
| Broadcast support                       | ✗                  | ✓                   | ✗                  |
| ISR-safe                                | ✓                  | Partial             | ✓                  |
| Suitable for ultra-low latency         | ✓                  | ✗                   | ✓                  |
| Strong type safety                      | Moderate           | High                | Moderate           |
| Works in task context                   | ✓                  | ✓                   | ✓                  |
| Works in ISR context                    | ✓                  | With caution        | ✓                  |
| Best for system-level signals           | ✓                  | ✓                   | ✓                  |
| Best for structured application logic   | ✗                  | ✓                   | ✗                  |

---

## Task Notifications

Task notifications are ideal for simple, low-overhead signaling between tasks. They are implemented using FreeRTOS's `xTaskNotifyIndexed` function and deliver a single integer value to a specific task. This mechanism is highly efficient and is suitable for use in both normal task execution and interrupt contexts.

### When to Use Task Notifications

- You need to notify a specific task of a state change.
- The signal contains no data, or only a small integer is required.
- You want the lowest possible latency and overhead.
- The notification must be ISR-safe.

### Example: Sending a Task Notification

```cpp
xTaskNotifyIndexed(taskHandle, 0,
                   Notification::NetworkReady.code(),
                   eSetValueWithOverwrite);
```

### Example: Waiting for a Task Notification

```cpp
void NetworkController::run() {
    waitFor(Notification::NetworkReady);
    printf("Network is ready\n");
}
```

---

## EventManager Events

The EventManager provides a flexible event-driven messaging system. It supports delivery of structured `Event` objects that include source and target information, notification metadata, and optional payloads.

This system is modeled after pub/sub and message-routing paradigms. It allows decoupled communication between different components or tasks in your application.

### Event Structure

Each `Event` consists of the following:

- A `Notification`, which distinguishes the type of event. This may be a system-level or user-defined notification.
- An optional source pointer to indicate who generated the event.
- An optional target pointer to direct the event to a specific recipient.
- An optional payload, passed as a pointer and size pair.

Events are delivered through task-safe queues and processed in the context of the receiving task, not from an ISR.

### When to Use EventManager Events

- You are sending structured data between components.
- You need multiple subscribers or want to broadcast.
- You want to separate producer and consumer logic.
- You require routing (e.g., targeting a specific controller).

### Example: Posting an Event with Payload

```cpp
Event evt;
evt.source = this;
evt.target = AppContext::get<ConfigController>();
evt.notification = Notification::makeUser(UserNotification::ConfigUpdated);
evt.data = &updatedConfig;
evt.size = sizeof(updatedConfig);

EventManager::postEvent(evt);
```

### Example: Receiving the Event

```cpp
void ConfigController::onEvent(const Event& event) {
    if (event.notification.kind == NotificationKind::User &&
        event.notification.user == UserNotification::ConfigUpdated) {
        auto* cfg = static_cast<ConfigData*>(event.data);
        applyNewConfig(*cfg);
    }
}
```

---

## GPIO Event Handling

The GPIO system in PicoFramework supports both direct listeners and event-based messaging. This dual-mode system provides the flexibility to use whichever mechanism best suits your needs.

### Options for GPIO Event Dispatch

You can configure how GPIO events are handled in `framework_config.h`. This determines whether GPIO changes trigger direct callbacks, post events to the `EventManager`, or both.

```cpp
#define GPIO_NOTIFICATIONS            1
#define GPIO_EVENTS                   2
#define GPIO_EVENTS_AND_NOTIFICATIONS (GPIO_NOTIFICATIONS | GPIO_EVENTS)
#define GPIO_EVENT_HANDLING           GPIO_EVENTS_AND_NOTIFICATIONS
```

This lets you choose fast, immediate handling, or structured message-based delivery, or both at the same time.

### Internal GPIO Handler Example

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

---

## Direct GPIO Listeners

GPIO listeners provide the fastest response to pin changes. These are direct callbacks registered per-pin that are invoked immediately when the corresponding GPIO edge event occurs.

Because they are often invoked from an interrupt context, they must be safe to run in that environment (e.g., no dynamic memory, no blocking calls).

### When to Use GPIO Listeners

- You need immediate response to pin changes.
- You are in an ISR context.
- You want to avoid the overhead of queuing and routing.

### Example: Adding a Listener

```cpp
gpioEventManager.addListener(pin, [](const GpioEvent& evt) {
    // React quickly to a pin state change
});
```

GPIO listeners can coexist with event-based GPIO handling if configured properly.

---

## Notification Typing and Safety

PicoFramework distinguishes between two notification domains:

- SystemNotification: used for framework-level signals like `NetworkReady`, `TimeSync`, or `GpioChange`.
- UserNotification: used for application-defined signals such as `ZoneStart`, `ProgramCompleted`, etc.

These are wrapped in a unified `Notification` object that uses a `NotificationKind` tag to differentiate between system and user domains. This ensures strong typing, prevents accidental collisions, and allows clean event routing.

### Example: Declaring Notification Enums

```cpp
enum class SystemNotification {
    NetworkReady = 1,
    StorageMounted = 2
};

enum class UserNotification {
    ProgramStarted = 1,
    ProgramStopped = 2
};
```

Even though values may overlap, the framework will correctly distinguish between them at runtime using the `NotificationKind` field.

---

## Summary Guidelines

The following recommendations will help you choose the right mechanism:

- Use Task Notifications if:
  - You need ultra-fast, one-to-one signals.
  - You are working in an ISR or task context.
  - No payload is required.
  - You want minimal overhead.

- Use EventManager if:
  - You need to send structured data.
  - You want to target specific controllers or broadcast.
  - You want to implement clean pub-sub communication.
  - You need to support more complex application logic.

- Use GPIO Listeners if:
  - You need fast GPIO response in interrupt context.
  - You do not require routing or structured messaging.
  - You are okay with static registration of callbacks.

Each mechanism is first-class and can be used individually or in combination to match your design needs. PicoFramework encourages clear separation of concerns and promotes flexible messaging patterns for embedded applications.
