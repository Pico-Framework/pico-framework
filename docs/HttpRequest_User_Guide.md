# PicoFramework: HttpRequest User Guide

---

## Overview

`HttpRequest` is the PicoFramework class that handles **incoming** and **outgoing** HTTP requests. It is primarily used **inside route handlers** to:
- Access incoming request details (method, path, headers, body)
- Handle uploaded files (multipart/form-data)
- Parse query strings and form fields
- Build and send outgoing client requests if needed

In normal use, the framework automatically passes an `HttpRequest` object to your route handlers.

You **do not manually call** `HttpRequest::receive()` unless you are building something low-level.

---

## Capabilities Summary

| Capability | Description |
|:-----------|:-------------|
| Access request data | Method, path, query params, headers, cookies, body |
| Handle uploads | Multipart file upload parsing |
| Fluent builder API (client-side) | Build and send outgoing HTTP requests |
| Save large bodies to file | Streaming body to storage if needed |

---

## Typical Server-Side Usage (in Routing)

In a route handler:

```cpp
void handlePostForm(HttpRequest& req, HttpResponse& res) {
    auto form = req.getFormParams();
    std::string username = form["username"];
    std::string password = form["password"];

    // Process form fields
    res.sendJson({{"status", "ok"}});
}
```

The framework automatically:
- Parses headers
- Parses body
- Parses query and form parameters
- Detects multipart uploads

No need to call `receive()` manually.

---

## Handling File Uploads

If the incoming request is a multipart form upload:

```cpp
void handleUpload(HttpRequest& req, HttpResponse& res) {
    if (req.isMultipart()) {
        req.handle_multipart(res);
    }
}
```

The multipart parser will:
- Extract uploaded files
- Store them to your storage
- Update form fields if present

---

## Using HttpRequest as a Client (optional)

You can use `HttpRequest` to send **outgoing** HTTP requests:

```cpp
HttpResponse res = HttpRequest()
    .setUri("https://api.example.com/data")
    .setMethod("GET")
    .setUserAgent("PicoFramework/1.0")
    .send();
```

Or for a POST:

```cpp
HttpResponse res = HttpRequest()
    .post("https://api.example.com/submit", "field1=value1&field2=value2");
```

Supported methods:
- `.get()`
- `.post()`
- `.put()`
- `.del()`

TLS (HTTPS) is supported if compiled with `PICO_HTTP_CLIENT_ENABLE_TLS`.

---

## Saving Incoming Request Bodies to File

If a large body is expected, instruct `HttpRequest` to store it to a file:

```cpp
req.toFile("/uploads/large_upload.bin");
```

This minimizes RAM usage during large uploads.

---

## Fluent API Summary (client-side)

| Method | Purpose |
|:-------|:--------|
| `.setUri(string)` | Set full or relative URI |
| `.setMethod(string)` | Set HTTP method (GET, POST, etc.) |
| `.setHeader(key, value)` | Set a single header |
| `.setHeaders(map)` | Set multiple headers |
| `.setBody(string)` | Set body content |
| `.setUserAgent(string)` | Set User-Agent header |
| `.setRootCACertificate(string)` | Set TLS CA certificate for HTTPS |
| `.toFile(string)` | Save incoming request body to file |

---

## Key Notes

- `HttpRequest` is passed automatically into route handlers.
- Parsing (headers, query, form) is automatic.
- Multipart file uploads are automatically detected.
- Client-side requests (optional) must call `.send()`.
- Large uploads can be redirected to storage with `.toFile()`.

---

## Next Steps

For how `HttpRequest` fits into **routing**, see the [Routing Guide] (separate document).

