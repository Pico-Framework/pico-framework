
# PicoFramework: Complete User Guide

This guide brings together everything you need to understand and use PicoFramework effectively. It covers:

- System structure and startup flow
- Core building blocks and how they relate
- Writing controllers, tasks, models, and views
- Using services and events
- Creating real applications

---

## Overview Overview

PicoFramework is a modular, event-driven framework for building robust, maintainable applications on embedded platforms. It is built around FreeRTOS and offers:

- Task-based concurrency using `FrameworkTask`
- Clean separation of logic into `FrameworkApp`, `FrameworkController`, `FrameworkModel`, and `FrameworkView`
- Central service management via `AppContext`
- Automatic system initialization through `FrameworkManager`
- HTTP server and routing integration out of the box

---

## Components Key Components and Responsibilities

### AppContext

`AppContext` is a global service registry. You use it to register and retrieve singleton services like `StorageManager`, `EventManager`, `TimeManager`, or your own custom services.

```cpp
AppContext::registerService<StorageManager>(&storage);
StorageManager* sm = AppContext::get<StorageManager>();
```

This avoids passing references manually and simplifies cross-module communication.

---

### FrameworkApp

`FrameworkApp` is your main application class. It:

- Inherits from `FrameworkController`
- Owns the `HttpServer` and `Router`
- Automatically integrates with `FrameworkManager` to bootstrap the system

You subclass `FrameworkApp` to define routes and launch your application.

```cpp
class MyApp : public FrameworkApp {
public:
    MyApp() : FrameworkApp(80, "MyApp") {}

protected:
    void initRoutes() override {
        router.addRoute("GET", "/", handleHome);
    }

    void handleHome(HttpRequest& req, HttpResponse& res, const RouteMatch&) {
        res.send("Hello, world!");
    }
};
```

Start your app using:

```cpp
MyApp app;
app.start();
```

---

### FrameworkManager

`FrameworkManager` handles system-level initialization. It is automatically created by `FrameworkApp` and performs:

- Starting the Wi-Fi or network stack
- Waiting for connectivity
- Running NTP-based time sync
- Posting a `SystemNotification::NetworkReady` event

You typically do not subclass or interact with `FrameworkManager` directly unless customizing system bootstrap logic.

---

### FrameworkController

`FrameworkController` is the base class for modular logic components. Each controller:

- Runs in its own FreeRTOS task
- Can subscribe to events via `onEvent(const Event&)`
- Can perform periodic work with `poll()`
- Can schedule repeated work with `runEvery()`

You can create multiple controllers to separate concerns like sensors, actuators, scheduling, etc.

```cpp
class SensorController : public FrameworkController {
public:
    SensorController() : FrameworkController("Sensor", sharedRouter) {}

protected:
    void onEvent(const Event& event) override { }
    void poll() override { }
};
```

---

### FrameworkTask

`FrameworkTask` is the base class for creating any FreeRTOS task in a consistent, event-aware way. It simplifies:

- Task creation (no need for xTaskCreate)
- Notifications (`notify()`, `waitFor()`)
- Optional message queues
- Suspending and resuming

```cpp
class MyTask : public FrameworkTask {
public:
    MyTask() : FrameworkTask("Worker", 2048, tskIDLE_PRIORITY + 1) {}

protected:
    void run() override {
        while (true) {
            if (waitFor(0, pdMS_TO_TICKS(5000))) {
                // Handle notification
            }
        }
    }
};
```

You can also send notifications or queue messages between tasks using `notify()`, `notifyFromISR()`, and the optional queue interface.

---

### FrameworkModel

`FrameworkModel` provides lightweight JSON-backed storage for persistent structured data. It supports:

- Loading and saving JSON arrays
- Creating, updating, and deleting objects
- Abstracted access to FatFs or LittleFs

```cpp
FrameworkModel zones(AppContext::get<StorageManager>(), "/zones.json");
zones.load();
zones.create({{"id", "zone1"}, {"name", "Front Lawn"}});
zones.save();
```

You can subclass it to define specific domain models.

---

### FrameworkView

`FrameworkView` provides:

- HTML template rendering with placeholder substitution
- JSON response formatting

It is ideal for building dashboards, status pages, and REST API responses.

```cpp
std::map<std::string, std::string> context = {
    {"title", "Status"},
    {"device", "Pico"}
};

res.send(FrameworkView::render("/www/index.html", context), "text/html");
```

Or return JSON:

```cpp
res.send(FrameworkView::renderJson(model.findAsJson("item1")), "application/json");
```

---

## Startup Flow Application Startup Sequence

1. `main()` or `boot()` creates your `FrameworkApp` subclass
2. `FrameworkApp::start()` calls `FrameworkManager` to:
   - Start networking
   - Wait for IP address
   - Sync system time
3. Core services like `StorageManager`, `EventManager`, `TimeManager` are registered in `AppContext`
4. Your application routes are set up via `initRoutes()`
5. Additional controllers and tasks can be started manually
6. System runs and begins handling HTTP requests and events

---

## Usage Writing Applications the PicoFramework Way

Here’s what goes where:

- **FrameworkApp**: Your application entry point. Owns `Router`, `HttpServer`, integrates startup.
- **initRoutes()**: Register all URL routes and handlers here.
- **FrameworkController**: Encapsulate device or feature logic into separate controllers.
- **onEvent()**: React to system or user events.
- **poll() / runEvery()**: For periodic or timed work.
- **FrameworkModel**: Define and persist structured data like programs or zones.
- **FrameworkView**: Render HTML or JSON for responses.

Use `AppContext` to access shared services from anywhere without tight coupling.

---

## Examples Example App Structure

```cpp
MyApp app;
SensorController sensor;
BlinkerTask blinker;

ZonesModel zones(AppContext::get<StorageManager>(), "/zones.json");

void MyApp::initRoutes() {
    router.addRoute("GET", "/zones", [&](HttpRequest& req, HttpResponse& res, const RouteMatch&) {
        res.send(FrameworkView::renderJson(zones.all()), "application/json");
    });
}

app.start();
```

---

 You now have everything you need to build modern, structured, and event-driven applications with PicoFramework.



## Expanded Details and Examples

### FrameworkApp - Application Entry Point

FrameworkApp is the base class you will subclass for your main application. It combines routing, HTTP server handling, and startup sequencing all in one place. It also acts as a controller itself, so you can handle system-level events here directly.

#### Extended Example

```cpp
class SprinklerApp : public FrameworkApp {
public:
    SprinklerApp() : FrameworkApp(80, "SprinklerApp") {}

protected:
    void initRoutes() override {
        router.addRoute("GET", "/", [this](HttpRequest& req, HttpResponse& res, const RouteMatch&) {
            std::map<std::string, std::string> context = {
                {"title", "Sprinkler Control"},
                {"status", "Ready"}
            };
            res.send(FrameworkView::render("/www/dashboard.html", context), "text/html");
        });

        router.addRoute("POST", "/api/zones/start", [this](HttpRequest& req, HttpResponse& res, const RouteMatch&) {
            // Extract JSON body or form data, validate, and start zone
            res.send("{"status":"started"}", "application/json");
        });
    }

    void onEvent(const Event& e) override {
        if (e.notification.isSystem(SystemNotification::NetworkReady)) {
            printf("Network is ready. Syncing config...
");
        }
    }
};
```

This example shows a realistic setup for both a status dashboard and an API handler. You can see how `initRoutes()` handles endpoint definitions and how `onEvent()` processes startup notifications.

---

### FrameworkManager - System Initialization

FrameworkManager is automatically constructed and used by FrameworkApp. Its role is to perform all system-level startup tasks so that your application logic can assume networking and time sync are ready.

#### What It Does

- Starts Wi-Fi or other network interfaces
- Waits for IP configuration (DHCP or static)
- Performs NTP time synchronization
- Posts a system event once networking is live

You typically do not need to customize FrameworkManager. However, if you do, you can subclass it and override its behavior. This is useful if you want to implement fallback logic, retry policies, or manual time sources.

#### Customization Tip

If you need to customize system services (e.g., register your own TimeManager), do so before calling `FrameworkApp::start()`:

```cpp
CustomTimeManager timeMgr;
AppContext::registerService<TimeManager>(&timeMgr);

SprinklerApp app;
app.start();
```

---

### FrameworkController - Modular Logic Blocks

Controllers are the best way to split your application into reusable logic modules. Each controller runs in its own task context and can respond to events, do periodic polling, and expose routes via the shared router.

#### Extended Example: ZoneController

```cpp
class ZoneController : public FrameworkController {
public:
    ZoneController(Router& r) : FrameworkController("ZoneCtrl", r) {}

protected:
    void initRoutes() override {
        router.addRoute("GET", "/api/zones", [this](HttpRequest& req, HttpResponse& res, const RouteMatch&) {
            res.send(FrameworkView::renderJson(zones.all()), "application/json");
        });

        router.addRoute("POST", "/api/zones/{id}/start", [this](HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
            std::string id = match.pathVars.at("id");
            startZone(id);
            res.send("{"status":"started"}", "application/json");
        });
    }

    void poll() override {
        // Monitor running zones
    }

    void onEvent(const Event& e) override {
        if (e.notification.isUser(UserNotification::ProgramStarted)) {
            printf("Program started
");
        }
    }

private:
    void startZone(const std::string& id) {
        // GPIO or scheduling logic here
    }

    FrameworkModel zones = FrameworkModel(AppContext::get<StorageManager>(), "/zones.json");
};
```

In this example:
- We expose multiple REST endpoints
- We use a model for persistent zone definitions
- We respond to user-defined program events
- We separate concerns by offloading zone logic from the main app

---

