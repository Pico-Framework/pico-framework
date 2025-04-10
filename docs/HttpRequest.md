## HttpClient and HttpRequest Usage Guide

### Overview

The framework provides a flexible HTTP client API using two primary classes:

- **`HttpRequest`** – Main user interface for building and sending HTTP requests
- **`HttpClient`** – Internal utility that performs the actual socket communication (used internally)

TLS (HTTPS), root CA verification, and chunked responses are supported. The client uses `TcpConnectionSocket` internally.

---

### Basic GET Request

```cpp
HttpResponse response = HttpRequest::create()
    .setUri("https://api.example.com/data")
    .setUserAgent("PicoFramework/1.0")
    .get();
```

---

### With Root CA Certificate

```cpp
extern const char* myCertPem; // Your PEM certificate

HttpResponse response = HttpRequest::create()
    .setUri("https://secure.example.com")
    .setRootCACertificate(myCertPem)
    .get();
```

---

### POST with Body and Custom Headers

```cpp
HttpRequest request = HttpRequest::create()
    .setUri("https://api.example.com/post")
    .setMethod("POST")
    .setHeader("Content-Type", "application/json")
    .setBody(R"({"device":"pico","status":"ok"})");

HttpResponse response = request.send();
```

---

### Save Response to File (If `StorageManager` Enabled)

```cpp
HttpResponse response = HttpRequest::create()
    .setUri("https://example.com/firmware.bin")
    .get();

response.saveFile("fw.bin"); // Uses active StorageManager
```

---

### Reuse `HttpRequest` for Multiple Calls

```cpp
HttpRequest request;
request.setHost("httpbin.org")
       .setProtocol("https")
       .setRootCACertificate(myCertPem)
       .setUserAgent("PicoFramework/1.0");

HttpResponse r1 = request.setUri("/get").get();
HttpResponse r2 = request.setUri("/status/200").get();
```

---

### Supported Methods

- `.get()`, `.post()`, `.put()`, `.delete_()`
- `.send()` – use when setting method manually

---

### Response Interface

```cpp
int status = response.getStatusCode();
std::string body = response.getBody();
std::string header = response.getHeader("Content-Type");

if (response.ok()) {
    // 200–299
}
```

---

### TLS Configuration

- Call `.setRootCACertificate(pem)` before `.send()`
- Hostname is automatically passed to TLS socket for SNI

---

### Notes

- Automatically adds `Host`, `Content-Length`, and `User-Agent` headers if not manually set
- Handles **chunked responses** automatically
- Uses **streamed recv** with memory-efficient parsing

