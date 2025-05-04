
# Logger in PicoFramework

Logging is essential in embedded systems for understanding what your application is doing, especially when something goes wrong. The PicoFramework includes a built-in `Logger` utility that supports both console output and persistent storage using the active `StorageManager`.

This guide explains:

- Why structured logging matters
- How the `Logger` works
- How to log to SD card or internal flash
- Log levels and filtering
- Real usage examples

---

## 1. Purpose and Design Goals

Embedded developers often rely on `printf()` for debug output, but it has limitations:

- No timestamps
- No severity levels
- No persistent storage (logs are lost on reset)

The `Logger` solves these problems with:

- Timestamps formatted as `YYYY-MM-DD HH:MM:SS`
- Log levels: INFO, WARN, ERROR
- Optional file logging via `StorageManager`
- Unified logging API throughout your application

---

## 2. Basic Usage

You can start using the logger without configuration. It will write to stdout:

```cpp
Logger::info("System initialized");
Logger::warn("Low battery detected");
Logger::error("Sensor read failed");
```

Sample output:

```
[2025-05-03 12:45:17] [INFO] System initialized
[2025-05-03 12:45:42] [WARN] Low battery detected
[2025-05-03 12:46:00] [ERROR] Sensor read failed
```

---

## 3. Enabling File Logging

To log messages to a file (e.g., `/log.txt` on SD card or flash), enable file logging and ensure a `StorageManager` is registered:

```cpp
Logger::enableFileLogging("/log.txt");
```

Now all log messages will be appended to the file as well as printed to stdout.

**Note:** Logging to file uses the same file API provided by `StorageManager`, so it works with both `FatFs` and `LittleFs`.

---

## 4. Setting Log Level

By default, the log level is `INFO`, which means `warn()` and `error()` will also appear.

You can raise the minimum log level to filter out less important messages:

```cpp
Logger::setMinLogLevel(LOG_WARN);  // Now info messages will be suppressed
```

This is useful for production builds where you may want to omit low-priority logs.

---

## 5. Integration with StorageManager

Logger uses the active `StorageManager` to append log entries to a file. This means:

- Logs are preserved across reboots
- You can download logs via a file browser route
- You can store logs on SD or flash depending on your configuration

Example:

```cpp
AppContext::register<StorageManager>(new FatFsStorageManager());
Logger::enableFileLogging("/log.txt");
Logger::info("Log system started");
```

The call to `appendToFile()` is safe and thread-aware, using FreeRTOS synchronization under the hood.

---

## 6. Thread Safety and Performance

Logger is designed to be safe in multi-task environments. However:

- Log messages are formatted in a temporary buffer (256 bytes max)
- Logging should not be used inside tight loops or ISRs
- File logging may fail silently if the storage backend is not ready or mounted

To optimize performance:

- Use `LOG_WARN` level or higher in production
- Avoid verbose logging in real-time sections

---

## 7. Adding Logger to Your Own Classes

While `Logger` is a singleton-style utility class, you can wrap it for clarity:

```cpp
class MyService {
public:
    void init() {
        Logger::info("MyService initialized");
    }
};
```

No additional setup is required beyond calling the static methods.

---

## 8. Summary

| Feature         | Supported |
|----------------|-----------|
| Timestamps      | Yes       |
| Log levels      | Yes       |
| Output to stdout| Yes       |
| Output to file  | Yes       |
| Thread-safe     | Yes       |
| Custom levels   | No        |
| Dynamic format  | No        |

Logger provides a structured, consistent, and portable way to record operational messages across your embedded system. It integrates with the rest of the PicoFramework seamlessly, with no external dependencies.

---

## Example: Full Setup

```cpp
AppContext::register<StorageManager>(new LittleFsStorageManager());
Logger::enableFileLogging("/system.log");
Logger::setMinLogLevel(LOG_INFO);

Logger::info("App started");
Logger::warn("Config file not found, using defaults");
Logger::error("SD card not mounted");
```



---

## 9. Log Rotation (Manual)

The built-in logger does not automatically rotate files, but you can implement manual rotation in your controller logic:

```cpp
void rotateLogIfNeeded() {
    auto* storage = AppContext::get<StorageManager>();
    size_t logSize = storage->getFileSize("/system.log");

    if (logSize > 50 * 1024) {  // 50 KB threshold
        storage->removeFile("/system.old.log");
        storage->renameFile("/system.log", "/system.old.log");
        Logger::enableFileLogging("/system.log");  // Reopen new log file
        Logger::info("Log rotated");
    }
}
```

You can run this periodically using a `FrameworkTask::runEvery()` or in a background poll loop.

---

## 10. Handling Logging Failures and Buffer Overflow

Logger uses a 256-byte internal buffer to format messages. If your log message (including timestamp and level) exceeds this length, it may be truncated.

### Best Practices:

- Avoid excessive log data in a single line
- Format JSON or structured data externally before logging
- Don't rely on Logger output inside tight loops or ISR contexts

File logging may fail silently if the storage is unmounted or full. To monitor logging health:

```cpp
if (!Logger::isFileLoggingEnabled()) {
    Logger::warn("Logging fallback to console only");
}
```

---

## 11. Web UI Log Viewer Integration

You can add a basic file viewer route to your app for remote log access:

```cpp
router.addRoute("GET", "/logs", [](HttpRequest& req, HttpResponse& res, const RouteMatch&) {
    std::vector<uint8_t> content;
    auto* storage = AppContext::get<StorageManager>();

    if (storage->readFile("/system.log", content)) {
        res.send(content, "text/plain");
    } else {
        res.send("Log file not found", "text/plain", 404);
    }
});
```

For a web-friendly UI, render it with line breaks and scrollable view:

```html
<pre style="white-space: pre-wrap; max-height: 500px; overflow-y: auto;">
  [Log content goes here]
</pre>
```

This allows developers and users to inspect logs without a terminal connection.

---

## 12. Summary of Enhancements

- Manual log rotation can be added using file rename + size check
- Max buffer is 256 bytes — long messages may be truncated
- Logger is best used in main loop and controller context, not ISRs
- File logging depends on mounted storage and can fall back silently
- Web UI integration gives insight without serial access

