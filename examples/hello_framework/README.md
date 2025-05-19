# hello_framework Example

This example demonstrates how to use the **PicoFramework** to create a fully functional HTTP server with dynamic routes and system event handling.

It is a highly commented and instructive example designed to:
- Showcase the full capabilities of `HttpRequest` and `HttpResponse`.
- Explain the usage of routes and route parameters.
- Illustrate how to process query strings, forms, and JSON payloads.
- Demonstrate system event handling (`NetworkReady`, `TimeValid`) via the `EventManager`.
- Provide a template for structuring real-world applications with the framework.

---

## Application Lifecycle

All applications in PicoFramework derive from `FrameworkApp`, which itself inherits from `FrameworkController`, and ultimately from `FrameworkTask`.

- **FrameworkTask**: wraps FreeRTOS tasks.
- **FrameworkController**: adds event handling and polling behavior.
- **FrameworkApp**: owns the HTTP router and server. It is the application's entry point.

The application task is created and managed by `FrameworkManager`, which:
- Starts the FreeRTOS scheduler.
- Initializes networking and emits system notifications.
- Populates the `AppContext` service registry (for shared services).

---

## Key Components in `App`

### `App::initRoutes()`

Defines HTTP routes using `router.addRoute(method, path, handler)`:
- Supports all major HTTP verbs: `GET`, `POST`, `PUT`, `DELETE`, `HEAD`, `OPTIONS`.
- Supports path parameters: `/update/{id}`
- Supports query strings: `/api/query?key=value`
- Supports JSON, form-urlencoded and raw bodies.
- Uses a catch-all route for unmatched GET requests.

### `HttpRequest` Usage Examples:
- `.getBody()` → retrieve raw request body.
- `.json()` → parse request body as JSON.
- `.getQueryParams()` → parse query string parameters.
- `.getFormParams()` → parse form data.
- `.getHeader("X")` and `.getHeaders()` → retrieve headers.

### `HttpResponse` Usage Examples:
- `.send(string)` → send plain text response.
- `.json(object)` → send JSON response.
- `.setHeader("X", "Y")` → add custom headers.
- `.status(404)` → set HTTP status code.

### Sample Routes
| Route                      | Method   | Purpose                                      |
|---------------------------|----------|----------------------------------------------|
| `/`                       | GET      | Simple response, prints request headers      |
| `/{name}`                | GET      | Path parameter example                       |
| `/api/data`              | GET      | JSON response                                |
| `/submit`                | POST     | Echo raw body                                |
| `/api/json`              | POST     | Parse and return JSON                        |
| `/api/form`              | POST     | Parse and return form-urlencoded data        |
| `/api/query`             | GET      | Echo query parameters                        |
| `/update/{id}`           | PUT      | Update simulation by ID                      |
| `/delete/{id}`           | DELETE   | Delete simulation by ID                      |
| `/api/header`            | GET      | Return User-Agent header                     |
| `/api/headers`           | GET      | Return all headers                           |
| `/api/custom`            | GET      | Return custom header and 202 status          |
| `/api/data`              | OPTIONS  | CORS preflight example                       |
| `/status`                | HEAD     | Respond with headers only                    |
| `*`                      | GET      | Catch-all, returns 404                       |

---

## System Event Handling

In `onStart()`:
- Calls `waitFor(NetworkReady)` to block until network is up.

In `onEvent()`:
- Reacts to `SystemNotification::NetworkReady`
- Handles `SystemNotification::TimeValid` to bootstrap schedulers.

---

## Polling Behavior

The method `poll()` is called periodically based on `getPollIntervalTicks()`.

```cpp
TickType_t App::getPollIntervalTicks() {
    return pdMS_TO_TICKS(100); // Customize poll interval per controller
}
```

For low-frequency polling or time-based tasks, use `runEvery()`:
```cpp
runEvery(1000, []() {
    std::cout << "[App] Polling..." << std::endl;
});
```

---

## AppContext

Provides access to globally registered services:
```cpp
AppContext::getInstance().get<EventManager>()->subscribe(...);
```

Also supports:
- `Logger` (provides a means of logging to serial or persistent storage log file)
- `JsonService` (persists JSON data through the storage interface)
- `StorageManager` (abstract interface to littlefs for flash or FAT+CLI for SD card)
- `TimerService` (provides both FreeRTOS timer services and services based on them such as scheduling at a defined Time of Day )

---

## Summary

This is the recommended example to study:
- All routes are clearly documented.
- It introduces every major concept used in web-based applications.
- It gives you full access to headers, payloads, events, and task behavior.
- For simpler onboarding, see the `minimum` example instead.

---

© 2025 Ian Archbell — MIT License
