## Routing and Controller Integration

Routing is at the heart of any HTTP server, and Pico-Framework brings a flexible and expressive routing model that’s inspired by modern web frameworks like Express.js. It allows embedded applications to define clear, readable HTTP endpoints that map directly to backend logic. This section covers:

- Route declaration syntax
- Path parameter handling
- Method-specific routing
- Default and wildcard routes
- Separation of routes into controllers
- Practical, real-world patterns

### Defining Routes

Routes are typically defined within a controller using the `router` instance, which is available via the base `FrameworkController` class. Routes can handle any HTTP method — `GET`, `POST`, `PUT`, `DELETE`, and others as needed — and are mapped to handler functions (usually lambdas or bound methods).

```cpp
router.get("/api/v1/status", [this](HttpRequest& req, HttpResponse& res) {
    return this->handleStatus(req, res);
});
```

This example sets up a simple GET /api/v1/status endpoint that routes to a controller method. The handler is passed both the HttpRequest and HttpResponse objects, allowing full control over input and output.

### Parameterized Routes

You can define routes with parameters using braces {}. These parameters are parsed and extracted automatically, and are available via req.getParam("name").

```cpp
router.get("/api/v1/zones/{name}", [this](HttpRequest& req, HttpResponse& res) {
    auto zone = req.getParam("name");
    return this->handleZoneQuery(zone, res);
});
```

You can define as many path parameters as needed:

```cpp
router.get("/users/{userId}/devices/{deviceId}", [this](HttpRequest& req, HttpResponse& res) {
    auto user = req.getParam("userId");
    auto device = req.getParam("deviceId");
    return this->getDeviceForUser(user, device, res);
});
```
These routes are evaluated in order of declaration, and parameter extraction is safe and type-agnostic (you receive a string, but can convert as needed).

### Handling Different HTTP Methods

Pico-Framework explicitly distinguishes between HTTP methods, so you can define the same path with multiple handlers:
```cpps
router.get("/api/v1/settings", [this](HttpRequest& req, HttpResponse& res) {
    return this->getSettings(req, res);
});

router.put("/api/v1/settings", [this](HttpRequest& req, HttpResponse& res) {
    return this->updateSettings(req, res);
});
```

Each method has its own routing table, making it easy to follow RESTful conventions.

### Wildcard and Fallback Routes

You can define a catch-all route to handle unmatched paths — commonly used for serving single-page apps or static content:

```cpp
router.get("/(.*)", [this](HttpRequest& req, HttpResponse& res) {
    return this->serveStaticFile(req, res);
});
```

This route matches anything not previously matched. You can still use req.uri() to see the original path and respond accordingly.

### Controller-Based Design

Controllers are classes derived from FrameworkController, and they encapsulate a group of related routes and behaviors. This approach brings structure and modularity to your embedded application.

Each controller implements the initRoutes() method, which is automatically invoked at startup by the framework.

```cpp
class DeviceController : public FrameworkController {
public:
    void initRoutes() override {
        router.get("/devices", [this](HttpRequest& req, HttpResponse& res) {
            return this->listDevices(req, res);
        });

        router.get("/devices/{id}", [this](HttpRequest& req, HttpResponse& res) {
            return this->getDevice(req.getParam("id"), res);
        });
    }
};
```

This keeps routes logically grouped, allowing your application to scale cleanly.

Controllers can also store shared state, access services, or post events via the AppContext and EventManager.

### Returning Responses from Handlers

Your handler functions can either:

* Return true to indicate the response was sent immediately
* Return false to defer response generation (e.g. when waiting on an event or background task)
* Call res.sendJson(), res.sendText(), res.sendFile(), etc., directly

Example:
```cpp
bool getDevice(const std::string& id, HttpResponse& res) {
    auto device = deviceService.lookup(id);
    if (!device) {
        return res.sendJsonError(404, "Device not found");
    }
    return res.sendJson({
        { "id", device->id },
        { "status", device->status },
    });
}
```

### Sharing Logic Across Routes

Controllers can define internal helper methods to process common logic across multiple routes:

```cpp
bool requireAuthenticated(HttpRequest& req, HttpResponse& res) {
    if (!req.isAuthenticated()) {
        return res.sendJsonError(401, "Unauthorized");
    }
    return false;
}
```

Then resuse it:

```cpp
router.get("/api/v1/secure-data", [this](HttpRequest& req, HttpResponse& res) {
    if (requireAuthenticated(req, res)) return true;
    return res.sendJson({ { "data", "secret" } });
});
```

### Summary

Routing in Pico-Framework is clean, expressive, and modular. It supports:

* Fine-grained control over path and method handling
* Easy parameter parsing
* REST-style endpoint structures
* Logical grouping of routes into controller classes
* Optional middleware and shared utility functions
* Together, these capabilities bring structure and clarity to your embedded HTTP application while remaining lightweight and efficient for microcontroller use.
