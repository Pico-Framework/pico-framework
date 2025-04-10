## HttpResponse Usage Guide

### Overview

`HttpResponse` represents the result of an HTTP request sent using `HttpRequest`. It provides access to status code, headers, and body content, and supports optional file saving if `StorageManager` is available.

---

### Basic Access

```cpp
HttpResponse response = request.get();

int status = response.getStatusCode();
bool ok = response.ok();
std::string body = response.getBody();
std::string contentType = response.getHeader("Content-Type");
```

---

### Check Status

```cpp
if (response.ok()) {
    // Status 200–299
} else {
    printf("Request failed: HTTP %d\n", response.getStatusCode());
}
```

---

### Access Headers

```cpp
std::string value = response.getHeader("X-Custom-Header");
if (!value.empty()) {
    printf("Custom header: %s\n", value.c_str());
}
```

---

### Save Response to File (If `StorageManager` Enabled)

```cpp
if (response.ok()) {
    response.saveFile("data.json");
}
```

---

### Clear and Reuse

You can reuse a single `HttpResponse` object across multiple requests:

```cpp
HttpRequest request = HttpRequest::create().setHost("example.com").setProtocol("https");
HttpResponse response;

request.setUri("/one").get(response);
request.setUri("/two").get(response); // response is reset internally
```

---

### Notes

- `getStatusCode()` returns the HTTP status integer (e.g., 200, 404).
- `ok()` is a shortcut for status code in the 200–299 range.
- `getHeader(key)` is case-insensitive and returns an empty string if not found.
- `saveFile(filename)` only works if file support is enabled in the build.
- Internally reset between calls to prevent stale values.

---

This class is typically not constructed directly; use it with `HttpRequest` for automatic population.