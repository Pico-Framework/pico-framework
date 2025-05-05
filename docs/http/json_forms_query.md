## JSON Handling, Query Parameters, and Form Parsing

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