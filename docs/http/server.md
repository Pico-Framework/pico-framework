# HTTP Server

## Introduction

The HTTP server built into Pico-Framework brings the power of modern web architectures to embedded systems. Inspired by platforms like Express.js, it provides a clean and extensible way to handle HTTP requests, define routes, manage controllers, and serve both static and dynamic content. Despite running on resource-constrained microcontrollers, it offers a robust and developer-friendly experience, enabling web-first designs even on devices like the Raspberry Pi Pico.

This document describes the full capabilities of the HTTP server and how it integrates seamlessly with the rest of the framework.

## Overview of Capabilities

- Declarative routing with parameter matching
- Fluent request and response interfaces
- Built-in support for JSON, form data, and file uploads
- Transparent handling of gzip-compressed assets
- Chunked and multipart response support
- Integration with `FrameworkController` for modular application logic
- Optional authentication middleware
- Automatic Content-Type and Content-Encoding detection
- Static file serving from SD card, LittleFS, or embedded assets

## Routing

Routes are defined within a controller using the `router` instance. Routes can handle standard HTTP methods (`GET`, `POST`, `PUT`, `DELETE`) and include parameterized paths.

Example:

```cpp
router.get("/api/v1/zones", [this](HttpRequest& req, HttpResponse& res) {
    return this->handleGetZones(req, res);
});

router.post("/api/v1/zones/{name}/start", [this](HttpRequest& req, HttpResponse& res) {
    return this->handleStartZone(req, res);
});
```

Route parameters (e.g. `{name}`) are parsed automatically and made available via `req.getParam("name")`.

## Integration with Controllers

Controllers in Pico-Framework inherit from `FrameworkController` and define their own routes. The framework automatically calls `initRoutes()` at startup, and the controller registers routes via its internal router.

Example:

```cpp
void MyController::initRoutes() {
    router.get("/status", [this](HttpRequest& req, HttpResponse& res) {
        return this->handleStatus(req, res);
    });
}
```

This separation of concerns keeps logic modular and maintains a clean structure even in complex applications.

## Static File Serving

The server supports serving static files from:

- SD cards using `FatFsStorageManager`
- Onboard flash via `LittleFsStorageManager`
- Compiled-in assets via `EmbeddedAssetManager`

Paths are mapped automatically. For example, a request to `/index.html` will look in storage or embedded assets for the corresponding file.

The default route fallback can be configured as:

```cpp
router.get("/(.*)", [this](HttpRequest& req, HttpResponse& res) {
    return this->serveStaticFile(req, res);
});
```

## Gzip Compression Support

The HTTP server automatically detects and sets the appropriate `Content-Encoding: gzip` header when serving `.gz` files such as compressed HTML, CSS, or JavaScript.

This is done by reading the magic number at the start of the file and does not rely on filename extensions or MIME types.

No special configuration is required—compressed files are served efficiently and transparently.

## JSON Handling

Incoming JSON is automatically parsed into a map-like structure and available via `req.getJson()`. Outgoing JSON responses are supported via:

```cpp
res.sendJson({
    { "status", "ok" },
    { "zone", "front-yard" }
});
```

If the body contains invalid JSON, a 400 response is generated automatically.

## Form and Query Parsing

URL query parameters are parsed into `req.getQueryParam("key")`.

Form fields (application/x-www-form-urlencoded) are parsed similarly with `req.getFormField("key")`.

## Multipart File Uploads

File uploads are handled with the built-in `MultipartParser`. Uploaded files are saved to the configured upload directory (default: `/uploads`).

Files are streamed to disk as they arrive, avoiding memory exhaustion on large uploads.

The route handler for uploads typically looks like:

```cpp
router.post("/api/v1/upload", [this](HttpRequest& req, HttpResponse& res) {
    return this->handleUpload(req, res);
});
```

Within `handleUpload`, you can inspect the uploaded filename and path.

## Chunked Transfer Encoding

The HTTP server can respond using chunked transfer encoding if the content size is not known in advance.

This is useful for streaming data from sensors or logs:

```cpp
res.beginChunked();
res.write("partial data...");
res.write("more data...");
res.endChunked();
```

## Authentication Middleware

If JWT authentication is enabled in the build, routes can be protected with:

```cpp
router.get("/auth", authMiddleware([this](HttpRequest& req, HttpResponse& res) {
    return this->handleAuthenticatedRequest(req, res);
}));
```

Only requests with a valid token in the `Authorization` header will reach the handler.

## Custom Error Responses

Errors such as 404 or 500 can be handled centrally by customizing `HttpResponse::sendError()` or intercepting unmatched routes.

You can send structured error responses with:

```cpp
res.sendJsonError(404, "File not found");
```

## Logging and Tracing

All incoming requests and error paths can be traced via the framework's logging system. Tracing can be enabled per module using `TRACE()` macros, and user-level logging via `Logger` captures application events.

## Summary

The Pico-Framework HTTP server combines power, simplicity, and flexibility in a compact embedded form. Whether serving gzipped HTML dashboards, handling dynamic JSON APIs, or streaming chunked data, it brings modern web capability to devices as small as the Pico. With routing modeled after Express.js and deep controller integration, it's easy to build modular and maintainable web applications on bare metal.

# Routing and Controller Integration

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

# JSON Handling, Query Parameters, and Form Parsing

Modern HTTP APIs often involve structured data exchange using JSON, as well as simpler use cases involving URL query parameters or form submissions. Pico-Framework provides built-in support for all three, allowing embedded applications to interact seamlessly with web clients, browsers, and APIs.

This section describes:

- Receiving JSON in HTTP requests
- Sending structured JSON responses
- Query parameter parsing (`?key=value`)
- Form data parsing (`application/x-www-form-urlencoded`)
- Validation patterns and error handling
- Examples for common embedded use cases

---

### Receiving JSON in Requests

When a client sends a request with `Content-Type: application/json`, the framework automatically parses the body into an internal JSON object and makes it available via:

```cpp
auto json = req.getJson();

```

This returns a lightweight wrapper that behaves like a key-value map. Example:

```cpp
router.post("/api/v1/zones", [this](HttpRequest& req, HttpResponse& res) {
    auto json = req.getJson();

    std::string name = json["name"];
    int duration = json["duration"];

    if (name.empty() || duration <= 0) {
        return res.sendJsonError(400, "Invalid request data");
    }

    zoneManager.schedule(name, duration);
    return res.sendJson({ { "status", "scheduled" } });
});
```

The framework provides safe access to keys, type coercion, and basic fallback handling.

If the incoming JSON is malformed or exceeds the maximum size, the framework responds automatically with:

```cpp
400 Bad Request
{"error":"Invalid JSON"}
```

### Sending JSON Responses

To send structured JSON back to the client, use res.sendJson():

```cpp
res.sendJson({
    { "zone", "front-lawn" },
    { "status", "active" },
    { "remaining", 120 }
});
```

This generates:

```http
HTTP/1.1 200 OK
Content-Type: application/json

{
  "zone": "front-lawn",
  "status": "active",
  "remaining": 120
}
```

You can return nested structures and arrays as well:

```cpp
res.sendJson({
    { "zones", {
        { "front", true },
        { "back", false }
    }},
    { "uptime", 9274 }
});
```

Use this for dashboards, configuration data, sensor output, or device state.

### Query Parameter Parsing

For GET requests with query strings like:
```http
GET /search?zone=front&active=true
```

The framework parses these into a map available via:
```cpp
req.getQueryParam("zone")      → "front"
req.getQueryParam("active")    → "true"
```

Example handler:

```cpp
router.get("/search", [this](HttpRequest& req, HttpResponse& res) {
    auto zone = req.getQueryParam("zone");
    auto active = req.getQueryParam("active");

    if (zone.empty()) {
        return res.sendJsonError(400, "Missing zone parameter");
    }

    bool isActive = (active == "true");
    return this->searchByZone(zone, isActive, res);
});
```
This enables quick, REST-style filtering with no body parsing.

### Form Field Parsing

For POST or PUT requests that use application/x-www-form-urlencoded, the framework automatically parses form fields into:

```cpp
req.getFormField("name")
```

This works just like query parameters but is used when fields are submitted in the request body.

Example:

```cpp
router.post("/config", [this](HttpRequest& req, HttpResponse& res) {
    auto ssid = req.getFormField("ssid");
    auto password = req.getFormField("password");

    if (ssid.empty()) {
        return res.sendJsonError(400, "Missing SSID");
    }

    wifiManager.configure(ssid, password);
    return res.sendJson({ { "status", "configured" } });
});
```
This is especially useful for browser-based setup forms.

### Validation and Error Handling

The framework provides a unified way to return errors via:

```cpp
res.sendJsonError(400, "Missing required field");
res.sendJsonError(401, "Unauthorized");
res.sendJsonError(500, "Internal error");
```

Use this to maintain consistent API responses and make frontend integration easier.

You can also define helper methods to validate required fields:

```cpp
bool requireJsonFields(HttpRequest& req, HttpResponse& res, std::vector<std::string> keys) {
    auto json = req.getJson();
    for (auto& key : keys) {
        if (json[key].empty()) {
            res.sendJsonError(400, "Missing field: " + key);
            return true;
        }
    }
    return false;
}
```

Then use it in routes:
```cpp
if (requireJsonFields(req, res, { "zone", "duration" })) return true;
```

Combined Use
The system is flexible enough to handle any combination of body formats:

* Use JSON for structured APIs
* Use query parameters for filtering
* Use form fields for browser-submitted forms

All are accessible via intuitive methods with no boilerplate parsing code required.

Summary
Pico-Framework makes it easy to receive and respond with structured data using:

* req.getJson() for JSON bodies
* req.getQueryParam() for query strings
* req.getFormField() for form data
* res.sendJson() and res.sendJsonError() for consistent responses

These capabilities are essential for building robust, interactive APIs and web interfaces on embedded devices.

# Static File Serving and Gzip Support

One of the most powerful and user-friendly capabilities of the Pico-Framework HTTP server is its built-in static file serving. It allows you to host full web applications, dashboards, or configuration UIs directly from the device, even under constrained storage conditions. These assets can be served from multiple storage backends and support compression transparently.

This section covers:

- Static file serving architecture
- Supported storage options (SD card, LittleFS, Embedded)
- MIME type detection
- Gzip file detection and automatic Content-Encoding
- Fallback routing for single-page apps
- Real-world example scenarios

---

### Serving Static Files from Storage

By default, the framework maps URI paths to the virtual filesystem used by your application. A request like:

GET /index.html

Will resolve to:

- `/index.html` on the mounted SD card if using `FatFsStorageManager`
- `/index.html` on flash if using `LittleFsStorageManager`
- `index.html` from compiled-in assets if using `EmbeddedAssetManager`

This logic is implemented by `HttpFileserver`, which maps paths and calls `res.sendFile()` with appropriate headers.

#### Example Route for Static Content:

```cpp
router.get("/(.*)", [this](HttpRequest& req, HttpResponse& res) {
    return res.sendFile(req.uri());
});
```

This catch-all fallback ensures that any non-API route falls back to static content handling, which is ideal for SPAs and dashboards.

#### Storage Backends
The file server supports three interchangeable storage systems:

* FatFsStorageManager
Uses an SD card via SPI or SDIO. Ideal for large apps or systems requiring removable storage.

* LittleFsStorageManager
Mounts a LittleFS volume in flash memory. Persistent, fast, and available even without external hardware.

These backends are selected via configuration and registered automatically by the framework at startup.

#### MIME Type Detection

The server uses built-in MIME type mapping to set the correct Content-Type headers based on the file extension:

**[this list needs updating]**
.html   → text/html
.css    → text/css
.js     → application/javascript
.png    → image/png
.svg    → image/svg+xml
.json   → application/json

This allows modern web apps to function correctly in browsers with no need for user configuration.

You can override or extend MIME types if needed by calling HttpFileserver::addMimeType().

### Automatic Gzip Compression Handling

The server supports serving pre-compressed .html, .css and .js files automatically generating the gzip Content-Encoding header. It determines whether to do this by checking the first two bytes of the file for the gzip "magic number"  (bytes 0x1f 0x8b).

```
Content-Encoding: gzip
```

This dramatically improves performance for larger pages and enables serving modern frontends like React or Vue apps.

### Single-Page Applications and Fallback Routing

For SPAs or dashboard-style UIs, you often want unknown routes to fall back to index.html. This can be done with:

```cpp
router.get("/(.*)", [this](HttpRequest& req, HttpResponse& res) {
    std::string path = req.uri();
    if (!res.sendFile(path)) {
        return res.sendFile("/index.html");
    }
    return true;
});
```
This allows client-side routers (e.g. Vue Router, React Router) to handle paths like /dashboard/settings, while the embedded server still serves the correct base file.

Note that regex expressions are precompiled so we can avoid exceptions at reuntime.

### Example: Full Static Content Setup

```cpp
void MyController::initRoutes() {
    // Serve API endpoints first
    router.get("/api/v1/device", [this](HttpRequest& req, HttpResponse& res) {
        return this->getDeviceInfo(req, res);
    });

    // Then fallback to file server
    router.get("/(.*)", [this](HttpRequest& req, HttpResponse& res) {
        if (!res.sendFile(req.uri())) {
            return res.sendFile("/index.html");
        }
        return true;
    });
}
```

This pattern ensures robust support for dynamic APIs and modern browser-based interfaces.

### Summary

The static file serving system in Pico-Framework enables full-featured web UIs on microcontrollers. With support for gzip compression, MIME types, flexible storage backends, and SPA fallback behavior, you can deploy everything from config pages to rich JavaScript dashboards with minimal effort and no custom code.

# Multipart File Uploads

File uploads are an essential capability in embedded web servers — enabling users to upload firmware, configuration files, media assets, or logs directly through the web interface. Pico-Framework provides built-in support for multipart file uploads using a streaming parser designed for memory efficiency.

This section covers:

- Multipart form uploads and their structure
- Memory-safe streaming to disk
- Configuring the upload directory
- Handling metadata and form fields
- Error handling and large file protection
- Practical use cases

---

### Upload Route Setup

To handle uploads, define a route that matches the incoming `POST` request. The framework automatically detects multipart content via the `Content-Type: multipart/form-data` header and invokes the `MultipartParser`. Uploaded files are stored in an /uploads directory on either flash or SD card depending on the StorageManager you are using. The default can be changed in framework_config.h.

```cpp
router.post("/api/v1/upload", [this](HttpRequest& req, HttpResponse& res) {
    return this->handleUpload(req, res);
});
```

The body is parsed and stored to disk as the data arrives — a single 1460 byet buffer is used and each buffer is appended the file enasuring that memory will not limit the upload.

### Uploading via Browser or curl

HTML form example:

```html
<form action="/api/v1/upload" method="post" enctype="multipart/form-data">
  <input type="file" name="file">
  <input type="submit" value="Upload">
</form>
```

curl example:

```bash
curl -F "file=@firmware.bin" http://device.local/api/v1/upload
```

### Multiple File Fields

The current parser supports one file field per upload request. If multiple files are needed, upload them individually, or implement multipart array parsing at the application level.

**[Example endpoint here]**

### Summary

Pico-Framework provides a clean, memory-safe approach to handling file uploads:

* Streams file data directly to disk
* Supports standard browser and tool uploads
* Protects RAM usage and handles large files
* Automatically parses multipart/form-data and extracts metadata
  
It enables embedded devices to receive firmware, images, or configuration data without requiring external servers or complex logic.

# Chunked Transfer Responses

Chunked transfer encoding is an HTTP feature that allows a server to begin sending a response before knowing its total size. This is especially useful in embedded applications where:

- Data is generated incrementally (e.g., from a sensor, log stream, or large file)
- The full content length is not known in advance
- The device has tight RAM constraints

Pico-Framework supports chunked responses out-of-the-box through the `HttpResponse` API.

This section covers:

- How chunked transfer works
- When to use it
- Fluent response interface
- Streaming sensor or event data
- Integrating with timers or background jobs
- Limitations and best practices

---

### How Chunked Transfer Works

In a chunked HTTP response, the server sends:

1. An initial response header with:
```html
Transfer-Encoding: chunked
```

2. Followed by one or more **chunks**, each prefixed by the size in hexadecimal:
```yaml
5\r\nHello\r\n
7\r\n world!\r\n
0\r\n\r\n
```


3. Ending with a zero-length chunk to mark the end.

This allows the client (e.g. browser, API consumer) to begin processing the response before the server finishes sending it.

---

### Starting a Chunked Response

To begin a chunked response:

**[check syntax]**

```cpp
res.beginChunked();  // Sends headers
```

Then send chunks one by one:

```cpp
res.write("Temperature: 23.4°C\n");
res.write("Humidity: 55%\n");
```

Finish with:

```cpp
res.endChunked();
```

This is especially useful in long-lived or time-based responses.

### Example: Streaming Sensor Data

```cpp
router.get("/stream/sensors", [this](HttpRequest& req, HttpResponse& res) {
    res.beginChunked();

    for (int i = 0; i < 10; ++i) {
        float temp = readTemperatureSensor();
        float humidity = readHumiditySensor();

        res.write("Reading " + std::to_string(i) + ": ");
        res.write("Temp=" + std::to_string(temp) + "C ");
        res.write("Hum=" + std::to_string(humidity) + "%\n");

        delay(1000); // simulate time delay or timer
    }

    res.endChunked();
    return true;
});
```

This allows the browser or client to receive updates line-by-line, even if the total content is long.

Real-Time Logging or Event Streams
Chunked responses also work well for:

* Returning system logs
* Pushing events in a stream
* Providing partial feedback during long operations
  
Example: returning logs:

```cpp
res.beginChunked();
for (auto& line : logBuffer) {
    res.write(line + "\n");
}
res.endChunked();
```

For real-time streams, consider wiring to a timer or event callback.

### Non-Blocking Response from Tasks

You may wish to delay between chunks or allow other tasks to run. Use a timer service or vTaskDelay() for pacing. Just ensure you retain control of the response object until endChunked() is called.

### Combining with JSON

You can also send partial JSON content, but the client must be aware it's receiving an open structure.

```cpp
res.beginChunked();
res.write("{\n");
res.write("  \"status\": \"streaming\",\n");
res.write("  \"data\": [\n");

for (int i = 0; i < 10; ++i) {
    res.write("    {\"value\": " + std::to_string(i) + "}");
    if (i != 9) res.write(",");
    res.write("\n");
}

res.write("  ]\n");
res.write("}\n");
res.endChunked();
```

This allows for lightweight telemetry streaming.

Best Practices
* Always call endChunked() to complete the response
* Don’t use sendJson() or sendText() after beginChunked()
* Avoid large buffers — stream in small parts
* Use with GET endpoints, or long-lived tasks
* Not all clients support chunked responses — test your integration
  
### Summary
Chunked transfer responses provide an efficient, memory-safe way to send large or dynamic output from embedded systems. Pico-Framework supports this natively with:

* beginChunked() to start the stream
* write() to send each chunk
* endChunked() to finalize
  
This makes it easy to serve real-time sensor feeds, logs, partial data, or progressive feedback in a fully compliant HTTP response stream.

