# PicoFramework: Routing and Server User Guide

---

## Overview

The PicoFramework routing system manages **incoming HTTP requests** and dispatches them to the appropriate **route handlers**. It also integrates:

- Static file serving (via `HttpFileserver`)
- Multipart upload handling (via `MultipartParser`)
- Full server management (via `HttpServer`)

You define routes declaratively and focus only on your application logic. File serving and uploads are built-in.

---

## Capabilities Summary

| Feature                 | Description                              |
| ----------------------- | ---------------------------------------- |
| Static routes           | Match exact paths like `/index.html`     |
| Dynamic routes          | Capture parameters like `/zones/{id}`    |
| Global middleware       | Apply middleware to all routes           |
| Per-route middleware    | Apply middleware only to specific routes |
| JWT Authentication      | Protect routes using Bearer tokens       |
| Built-in /auth endpoint  | Helper route for testing authentication  |
| Static file serving      | Serve files from storage automatically   |
| Multipart file uploads   | Handle large file uploads to storage     |
| JSON request helpers     | Simplify extracting fields from JSON     |
| JSON response helpers    | Standardize API success/error replies    |

---

## Server Structure

The `HttpServer` runs a dedicated FreeRTOS task:

- Listens for TCP connections
- Accepts incoming clients
- Parses the HTTP request (`HttpRequest`)
- Dispatches the request through the `Router`
- Serves static files if no route matches

It supports multiple clients (configurable via `MAX_CONCURRENT_CLIENTS`).

---

## How Routes Work

Each route maps an **HTTP method** (GET, POST, etc.) and a **path** to a **handler function**.

Handler signature:

```cpp
void myHandler(HttpRequest& req, HttpResponse& res, const RouteMatch& match)
```

No need to manually receive or parse the request — it is fully prepared.

---

## Defining a Basic Route

```cpp
router.addRoute("GET", "/zones", [](HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
    res.sendJson({{"message", "Listing all zones"}});
});
```

---

## Using Dynamic Parameters

```cpp
router.addRoute("GET", "/zones/{id}", [](HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
    std::string zoneId = match.named.at("id");
    res.sendJson({{"zone_id", zoneId}});
});
```

---

## Working with JSON Request Bodies

To extract fields from incoming JSON payloads easily, use `JsonRequestHelper`:

```cpp
#include "http/JsonRequestHelper.h"

void handleJsonRequest(HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
    std::string username = JsonRequestHelper::getString(req, "user.name");
    int age = JsonRequestHelper::getInt(req, "user.age", 0);

    res.sendJson({{"username", username}, {"age", age}});
}
```

Supported extractors:
- `getString()`, `getInt()`, `getDouble()`, `getBool()`
- `getArray()`, `getObject()`

---

## Responding to Requests

You typically use `HttpResponse` to reply:

```cpp
res.status(200).send("OK");
```

Or use `JsonResponse` helpers for clean API responses:

```cpp
#include "http/JsonResponse.h"

JsonResponse::sendSuccess(res, { {"key", "value"} }, "Operation successful");

// Or an error:
JsonResponse::sendError(res, 404, "NOT_FOUND", "Item not found");
```

Common JsonResponse helpers:
- `sendSuccess()`
- `sendCreated()`
- `sendError()`
- `sendNoContent()`

This keeps your API replies consistent and lightweight.

---

## Middleware

Middleware functions run **before** your route handler. They can:

- Inspect the request
- Block processing (e.g., authentication failure)
- Modify the request or response

Apply globally:

```cpp
router.use(loggingMiddleware);
```

Apply per-route:

```cpp
router.addRoute("POST", "/admin", handler, {authMiddleware});
```

---

## JWT Authentication Support

Built-in middleware `authMiddleware` validates Bearer tokens using `JwtAuthenticator`.

Example:

```cpp
router.addRoute("POST", "/api/secure", secureHandler, {authMiddleware});
```

Only valid tokens are allowed to reach the handler.

The `/auth` route is provided for testing Authorization headers.

---

## Static File Serving

If no route matches, the `HttpFileserver` automatically attempts to serve the request from storage.

Behavior:
- `/` maps to `/index.html`
- Other paths map to their filename
- MIME types are detected automatically
- Gzipped files are detected by magic number

Manual serving inside a route:

```cpp
fileServer.handle_static_request(req, res, match);
```

---

## Directory Listing API

The special `/api/v1/ls` endpoint lists directory contents:

- Returns JSON list of files and directories
- Used for file managers, upload managers, etc.

Example route:

```cpp
router.addRoute("GET", "/api/v1/ls{path}", [](HttpRequest& req, HttpResponse& res, const RouteMatch& match) {
    fileServer.handle_list_directory(req, res, match);
});
```

---

## Multipart File Uploads

When a client POSTs with `Content-Type: multipart/form-data`, the `MultipartParser` handles file upload:

- Detects boundaries
- Streams large files incrementally
- Writes directly to storage (not to RAM)
- Prevents duplicate filenames
- Only one file per upload is currently supported

Handling inside a route:

```cpp
if (req.isMultipart()) {
    req.handle_multipart(res);
}
```

Uploads are saved into the configured upload directory.

---

## Quick Reference: Core Components

| Component         | Purpose                                    |
| ----------------- | ------------------------------------------ |
| `HttpServer`      | Accepts TCP clients, dispatches requests   |
| `Router`          | Matches routes and applies middleware      |
| `HttpFileserver`  | Serves static files and directory listings |
| `MultipartParser` | Handles multipart/form-data uploads        |
| `JsonRequestHelper` | Simplifies JSON field extraction from requests |
| `JsonResponse`      | Standardizes sending JSON responses       |

---

## Full Lifecycle (Request Flow)

1. `HttpServer` accepts a TCP connection.
2. `HttpRequest` is parsed automatically.
3. Global middleware runs.
4. Route is matched.
5. Per-route middleware runs.
6. Handler is called if authorized.
7. If no match, `HttpFileserver` attempts static serving.
8. Multipart uploads are parsed if needed.

If any middleware returns `false`, request handling stops immediately.

---

## Final Notes

- **No need to parse TCP manually** — everything flows through `HttpRequest`/`HttpResponse`.
- **Static files** and **uploads** are automatically integrated.
- **TLS** is supported in client mode (`HttpRequest`) if enabled.
- **FreeRTOS** is used for task concurrency.
- **Lightweight design**: no dynamic heap fragmentation.

---


