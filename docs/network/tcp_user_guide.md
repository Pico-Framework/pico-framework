
# Tcp Class - Unified TCP and TLS Socket Abstraction

## Purpose and Design Rationale

The `Tcp` class provides a clean abstraction over lwIP TCP sockets, with optional mbedTLS encryption. It is used internally by the framework but also available for users who need to implement direct socket communication using protocols other than HTTP (e.g., MQTT, FTP, or custom protocols).

Reasons for using `Tcp` instead of raw sockets:

- Provides a consistent API for both plain and TLS sockets
- Enables future backends (e.g., alternative network stacks or test mocks)
- Encourages correct use of FreeRTOS-safe socket operations
- Allows reuse by users in custom protocols or for testing
- Integrates TLS setup with SNI and root CA verification

## Basic Usage (Plain TCP)

```cpp
Tcp socket;
if (!socket.connect("example.com", 1883, false)) {
    printf("Failed to connect\n");
    return;
}

socket.send("Hello", 5);

char buffer[64];
int len = socket.recv(buffer, sizeof(buffer));
```

## Secure TLS Usage

```cpp
Tcp socket;
socket.setRootCACertificate(myCertPem);
socket.setHostname("api.example.com");

if (!socket.connect("api.example.com", 443, true)) {
    printf("TLS connection failed\n");
    return;
}

socket.send(requestData, strlen(requestData));
int len = socket.recv(responseBuffer, sizeof(responseBuffer));
```

## Graceful Disconnect and Reuse

```cpp
socket.disconnect();

if (socket.connect("newhost.com", 80, false)) {
    // Connection reused for another request
}
```

## Design Highlights

- Supports blocking `connect()`, `send()`, and `recv()` methods
- Handles DNS resolution
- SNI and certificate verification built in for TLS
- Uses lwIP and mbedTLS under the hood
- Intended for advanced users building non-HTTP clients
- Safe to call from FreeRTOS tasks

## Limitations

- Only client-side TLS is supported in v1
- No automatic retry or reconnect
- No internal buffering — user must handle all chunking

## Integration

- Used internally by `HttpClient` and `HttpRequest`
- Can be used directly for protocols like MQTT, WebSockets, FTP, or custom binary protocols


## Error Handling and Best Practices

All methods return either a boolean (success/failure) or number of bytes read/written.

### Checking Connection Success

```cpp
if (!socket.connect("host", 1234, false)) {
    printf("Connection failed. Check DNS or host availability.\n");
    return;
}
```

### Handling Partial Sends and EOF

- `send()` blocks until all data is written or fails.
- `recv()` returns 0 if the remote side closes the connection.

```cpp
int len = socket.recv(buffer, sizeof(buffer));
if (len == 0) {
    printf("Remote side closed connection\n");
} else if (len < 0) {
    printf("Socket error during recv\n");
}
```

### Timeouts

Currently, socket operations are blocking and may wait indefinitely. Timeout support may be added in a future version. Users are encouraged to implement their own timeouts using FreeRTOS timers or polling.

---

## Using Tcp for Custom Protocols

The `Tcp` class can be used directly for binary protocols such as MQTT or even text-based ones like SMTP.

### Example: Custom Command Protocol

```cpp
Tcp socket;
if (socket.connect("device.local", 12345, false)) {
    const char* cmd = "SET MODE=AUTO
";
    socket.send(cmd, strlen(cmd));

    char response[32];
    int len = socket.recv(response, sizeof(response));
    response[len] = '\0';

    printf("Received response: %s\n", response);
}
```

---

## Planned Server-Side Support

The current version only supports client-side connections. Server functionality is under consideration for future releases and will include:

- `TcpServer` abstraction to bind, listen, and accept connections
- Optional TLS handshake using altcp TLS backend
- Connection callbacks with accepted `Tcp` instances
- Reuse of internal poll-based `recv()` logic

Users interested in server features should isolate socket access using `Tcp` where possible to enable smooth migration.

---

## Summary

Use `Tcp` when:

- You need direct socket access (e.g., MQTT)
- You want a consistent TLS and non-TLS interface
- You are writing unit-testable or backend-agnostic code
- You are building a high-performance socket client

Avoid `Tcp` if:

- You just need HTTP(S) — use `HttpRequest` instead
- You need advanced socket management (timeouts, select/poll) — consider building on top
