# PicoFramework: User Framework Guide

---

## ✨ Overview

The PicoFramework User Framework provides a clean, modular structure for building embedded applications.
It organizes your application into services, controllers, models, views, and a managing task layer —
making it easy to create scalable and maintainable device software.

The key user-facing components are:
- `AppContext` — Service registration and lookup
- `FrameworkApp` — Main application entrypoint
- `FrameworkController` — Modular event-driven controllers
- `FrameworkManager` — System bootstrap and network initialization
- `FrameworkModel` — JSON-backed data models
- `FrameworkView` — Lightweight HTML/JSON rendering

---

## 🔍 AppContext

`AppContext` provides a global registry for core services.
It allows services like `StorageManager`, `EventManager`, `TimeManager`, and `JwtAuthenticator` to be registered once and accessed globally.

### Key Usage

Register services during system startup:

```cpp
AppContext::registerService<StorageManager>(&storage);
```

Retrieve services anywhere:

```cpp
StorageManager* storage = AppContext::get<StorageManager>();
```

This avoids passing pointers manually through the system.

---

## 🚀 FrameworkApp

`FrameworkApp` is your main application task.
It:
- Inherits from `FrameworkController`
- Initializes its own `Router` and `HttpServer`
- Integrates with `FrameworkManager` for system startup

### Typical Usage

Subclass it to create your application:

```cpp
class MyApp : public FrameworkApp {
public:
    MyApp() : FrameworkApp(80, "MyApp") {}

protected:
    void initRoutes() override {
        router.addRoute("GET", "/", handleHome);
    }

    void handleHome(HttpRequest& req, HttpResponse& res, const RouteMatch&) {
        res.send("Hello world!");
    }
};
```

Call `start()` to launch:

```cpp
MyApp app;
app.start();
```

---

## 💪 FrameworkController

`FrameworkController` provides modular, event-driven logic.
You can define multiple controllers for different concerns (e.g., sensors, networking).

Each controller:
- Runs in its own FreeRTOS task
- Handles pub/sub events from `EventManager`
- Can poll periodically with `poll()`
- Can run timed functions easily with `runEvery()`

### Typical Controller

```cpp
class SensorController : public FrameworkController {
public:
    SensorController() : FrameworkController("Sensor", sharedRouter) {}

protected:
    void onEvent(const Event& event) override {
        // Handle events
    }

    void poll() override {
        // Background work
    }
};
```

---

## 🚂 FrameworkManager

`FrameworkManager` handles system initialization:
- Starts the Wi-Fi stack (or other networking)
- Waits for network readiness
- Initializes time synchronization (via NTP)
- Notifies the application when the network is ready

### Automatic Bootstrapping

When you create your `FrameworkApp`, it internally creates a `FrameworkManager`.
This manager will automatically:
- Start networking
- Sync time
- Notify the app via `notify(SystemNotification::NetworkReady)`

No user code needed unless you customize startup.

---

## 📚 FrameworkModel

`FrameworkModel` provides lightweight persistent storage for JSON arrays.

- CRUD operations for structured data
- Abstracts underlying storage (FatFs or LittleFs)
- Designed for small memory footprints

### Example Usage

```cpp
FrameworkModel zones(AppContext::get<StorageManager>(), "/zones.json");
zones.load();
zones.create({{"id", "zone1"}, {"name", "Front Lawn"}});
zones.save();
```

You can extend `FrameworkModel` to create app-specific models easily.

---

## 🖼️ FrameworkView

`FrameworkView` provides simple dynamic HTML template rendering:
- Loads an HTML file
- Replaces `{{key}}` placeholders with values from a context map
- Supports rendering JSON as pretty-printed output

### Example Usage

```cpp
std::map<std::string, std::string> context = {
    {"title", "Dashboard"},
    {"user", "Alice"}
};

std::string page = FrameworkView::render("/www/index.html", context);
res.send(page, "text/html");
```

For JSON:

```cpp
res.send(FrameworkView::renderJson(model.findAsJson("item1")), "application/json");
```

---

## 🔄 How It All Fits Together

1. `FrameworkManager` starts the system.
2. `AppContext` registers core services.
3. `FrameworkApp` runs your application logic.
4. `FrameworkController` manages modular subsystems.
5. `FrameworkModel` provides persistent storage.
6. `FrameworkView` renders HTML/JSON responses.

All glued together through events and services.


---

## 📈 Typical PicoFramework Application Structure

```cpp
MyApp app; // Inherits FrameworkApp
SensorController sensor; // Inherits FrameworkController
ZonesModel zones; // Inherits FrameworkModel

// In app initRoutes():
router.addRoute("GET", "/zones", [](HttpRequest& req, HttpResponse& res, const RouteMatch&) {
    res.send(FrameworkView::renderJson(zones.all()), "application/json");
});

app.start();
```

FrameworkManager automatically handles networking and system setup.
You focus only on your app logic.

---

✅ **You are now ready to build full applications with the PicoFramework User Framework!**

