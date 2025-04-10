## TcpConnectionSocket Usage Guide

### Overview

`TcpConnectionSocket` provides a unified abstraction for both plain and TLS-based TCP communication. It supports:

- Client usage (server support planned)
- Secure TLS (client-side only in v1)
- Hostname resolution with SNI for TLS
- Blocking send/receive with FreeRTOS support

Used internally by `HttpClient`, but can also be used directly for lower-level protocols.

---

### Basic TCP Connection

```cpp
TcpConnectionSocket socket;
if (!socket.connect("example.com", 80, false)) {
    printf("Connection failed\n");
    return;
}

socket.send("Hello", 5);
char buffer[64];
int len = socket.recv(buffer, sizeof(buffer));
```

---

### TLS (HTTPS) Connection

```cpp
TcpConnectionSocket socket;
socket.setRootCACertificate(myCertPem);
socket.setHostname("secure.example.com"); // for SNI

if (!socket.connect("secure.example.com", 443, true)) {
    printf("TLS connection failed\n");
    return;
}

socket.send(requestData, strlen(requestData));
int len = socket.recv(responseBuffer, sizeof(responseBuffer));
```

---

### Disconnect and Reuse

```cpp
socket.disconnect();

if (socket.connect("another.com", 80, false)) {
    // Reused
}
```

---

### v1 Feature Summary

- [x] **Client-side TLS (HTTPS)** with SNI and root CA verification
- [x] **Plain TCP** support
- [x] Unified `connect(host, port, useTls)` interface
- [x] Clean `send()` and blocking `recv()`
- [x] Graceful disconnect and reuse
- [x] DNS resolution built-in
- [x] Lightweight and safe for FreeRTOS/lwIP environments
- [x] No internal buffering — caller has full control
- [x] Fully integrated into `HttpClient` and `HttpRequest`

> ❗ Server-side TLS is not supported in v1.

---

### Notes

- `connect()` is blocking; it handles DNS lookup and socket setup.
- `recv()` is blocking until data is received or the socket closes.
- TLS requires root certificate and hostname for verification.
- Data is not buffered internally; manage chunking externally if needed.
- Intended for internal and advanced use. Use `HttpRequest` for typical HTTP.

---

This class underpins all network I/O in the framework and is designed for testability, flexibility, and secure embedded use.

