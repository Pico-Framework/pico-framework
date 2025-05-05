## Chunked Transfer Responses

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

