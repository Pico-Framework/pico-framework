
# PicoFramework Overview

PicoFramework is a modern, event-driven application framework designed for embedded systems like the Raspberry Pi Pico W. It brings high-level structure to bare-metal and FreeRTOS-based systems, while remaining modular, portable, and efficient. This document provides a high-level overview of all key components and how they fit together, based on the complete suite of user guides.

---

## Core Concepts

At the heart of PicoFramework is the idea of structured, service-oriented embedded programming. Applications are composed of tasks, controllers, services, and views—all coordinated through FreeRTOS and a lightweight router/event system.

The framework promotes:
- Loose coupling via service registration
- Modular design using `FrameworkController`
- Fully event-driven flow using `EventManager`
- Persistent storage and structured logging
- Declarative scheduling and clean routing

Everything is designed to work in resource-constrained environments with optional features enabled via modular configuration.

---

## Application Structure

Your main application derives from `FrameworkApp`, which:
- Sets up the HTTP server and router
- Registers services like `StorageManager`, `JsonService`, and `Logger`
- Starts system components like `FrameworkManager`
- Initializes all user-defined routes

Controllers (`FrameworkController`) implement discrete logic (e.g., GPIO control, scheduling, or file management). They handle requests, events, and background polling in an isolated task context.

---

## Key Services and Subsystems

### 1. **Storage System**

The `StorageManager` interface abstracts persistent file access:
- `FatFsStorageManager` for SD cards (large, removable, compatible)
- `LittleFsStorageManager` for flash (always available, small-footprint)

On top of this:
- `JsonService` persists flat JSON config and settings
- `FrameworkModel` stores and retrieves structured object arrays (e.g., zones or programs)

This architecture allows your application to remain independent of the storage medium.

### 2. **HTTP Server and Routing**

The framework includes a minimalist HTTP server with routing, request/response objects, and multipart/form parsing.

Routes are registered via:

```cpp
router.addRoute("GET", "/status", [](HttpRequest& req, HttpResponse& res, const RouteMatch&) {
    res.send("OK");
});
```

Static file serving and uploads are supported when `StorageManager` is enabled.

### 3. **Event and Notification System**

The `EventManager` provides indexed task notifications via `onEvent()` handlers in controllers. It supports:
- System notifications (e.g., network ready, time valid)
- User-defined event enums
- Publish/subscribe model via `subscribe()` and `postEvent()`

This separates concerns cleanly and avoids direct task-to-task coupling.

### 4. **Time and Scheduling**

The `TimeManager` handles:
- SNTP time sync
- Timezone detection via Open-Meteo
- AON/RTC initialization
- Time formatting and validity checks

The `TimerService` lets you schedule one-shot, recurring, or daily events using:
- `scheduleAt()`
- `scheduleEvery()`
- `scheduleDailyAt()`
- `scheduleDuration()`

All timers post events—not callbacks—maintaining the event-driven design.

### 5. **Logging**

The `Logger` provides timestamped, level-based logging:
- Console + optional file logging
- Thread-safe
- Compatible with all storage types
- Viewable via web routes

Advanced use includes manual log rotation and embedded log viewers.

### 6. **Networking and TCP**

Networking is initialized by the `Network` class with:
- Retry logic
- Status polling
- LED feedback
- Reconnection helpers

The `Tcp` class abstracts both TLS and plain sockets for client-side communication, enabling:
- Secure APIs
- MQTT or FTP-style protocols
- Lightweight, testable client code

### 7. **User Interface and Views**

The `FrameworkView` supports rendering HTML files with variable substitution or returning structured JSON using:

```cpp
res.send(FrameworkView::render("/index.html", context));
res.send(FrameworkView::renderJson(object));
```

Combined with static file support, this enables lightweight frontends hosted entirely from the device.

---

## Development Approach

- Register all services and models at startup using `AppContext::register<T>()`
- Define routes in `initRoutes()`
- Use `onEvent()` to respond to notifications
- Use `poll()` in each controller for periodic logic
- Keep logging minimal but structured

The framework is event-first, file-aware, and task-structured by default.

---

## Summary

PicoFramework gives embedded developers:

- A clean and testable architecture
- Structured persistence and logging
- A full HTTP server and JSON stack
- Safe task scheduling and timer-driven events
- Web-capable APIs and file hosting

It is ideal for connected, sensor-driven, or user-interfaced devices like automation controllers, monitoring systems, or IoT prototypes.

