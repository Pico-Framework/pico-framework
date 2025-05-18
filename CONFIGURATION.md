# Configuration Guide

PicoFramework allows users to override default system configurations via a user-provided `framework_configuration_user.h` file. This document explains how the configuration system works and lists all configurable options available in `framework_config_system.h`.

---

## 🔧 How to Override Configuration

Create a `framework_configuration_user.h` file in your project’s `include/` directory. Any settings defined here will override the system defaults from `framework_config_system.h`.

**Example:**

```cpp
#pragma once

#define WIFI_MAX_RETRIES 3
#define MULTIPART_UPLOAD_PATH "/my_uploads"
#define TRACE_HttpServer 1
```

Your application must ensure this file is included before any framework headers.

---

## ⚙️ Configurable Options

Below is a complete list of configuration options you may override:

### 🕒 Time and NTP

| Macro | Default | Description |
|-------|---------|-------------|
| `NTP_TIMEOUT_SECONDS` | 15 | Max wait time for SNTP sync |
| `DETECT_LOCAL_TIMEZONE` | 0 | Set to 1 to auto-detect timezone |

---

### 📶 Wi-Fi Configuration

| Macro | Default | Description |
|-------|---------|-------------|
| `WIFI_RETRY_TIMEOUT_MS` | 15000 | Wait time between Wi-Fi retries (ms) |
| `WIFI_MAX_RETRIES` | 5 | Maximum retry attempts |
| `WIFI_REBOOT_ON_FAILURE` | `false` | Reboot after max retries |
| `WIFI_MONITOR_INTERVAL_MS` | 30000 | Interval for Wi-Fi status checks (ms) |

---

### 📤 File Uploads

| Macro | Default | Description |
|-------|---------|-------------|
| `MULTIPART_UPLOAD_PATH` | `/uploads` | Upload directory path |

---

### 🌐 HTTP Configuration

| Macro | Default | Description |
|-------|---------|-------------|
| `MAX_HTTP_BODY_LENGTH` | 16 KB (RP2040), 4 KB (RP2350) | Max in-memory HTTP body |
| `HTTP_IDLE_TIMEOUT` | 500 | Timeout for idle connections (ms) |
| `HTTP_RECEIVE_TIMEOUT` | 2000 | Timeout for receive operations (ms) |
| `HTTP_BUFFER_SIZE` | 1460 | Buffer size for request/response |
| `STREAM_SEND_DELAY_MS` | 20 | Delay between stream sends (ms) |

---

### 🔌 GPIO Event System

| Macro | Default | Description |
|-------|---------|-------------|
| `ENABLE_GPIO_EVENTS` | (defined) | Enable GPIO event system |
| `GPIO_NOTIFICATIONS` | 1 | Use task notifications |
| `GPIO_EVENTS` | 2 | Use queued events |
| `GPIO_EVENTS_AND_NOTIFICATIONS` | `3` | Use both notifications and events |
| `GPIO_EVENT_HANDLING` | `GPIO_EVENTS_AND_NOTIFICATIONS` | Select event handling method |

---

### 🐛 Debug Trace System

| Macro | Default | Description |
|-------|---------|-------------|
| `QUIET_MODE` | (undefined) | Define to silence normal output |
| `QUIET_PRINTF(...)` | `printf(...)` | Macro for conditional output |
| `TRACE_USE_TIMESTAMP` | 1 | Show HH:MM:SS timestamps |
| `TRACE_USE_SD` | 0 | Enable SD card trace logging |
| `TRACE_LOG_PATH` | `/framework_trace.log` | SD trace log file |
| `TRACE_LEVEL_MIN` | `TRACE_LVL_INFO` | Minimum trace level (INFO, WARN, ERROR) |

---

### 📁 Per-Module Tracing Flags

All of the following are `0` by default. Set to `1` to enable trace output:

```
TRACE_AppContext
TRACE_ChunkedDecoder
TRACE_FatFsStorageManager
TRACE_FrameworkApp
TRACE_FrameworkController
TRACE_FrameworkModel
TRACE_FrameworkTask
TRACE_FrameworkManager
TRACE_HttpClient
TRACE_HttpFileserver
TRACE_HttpParser
TRACE_HttpRequest
TRACE_HttpResponse
TRACE_HttpServer
TRACE_Middleware
TRACE_JsonParser
TRACE_JsonRequestHelper
TRACE_JsonService
TRACE_JwtAuthenticator
TRACE_LittleFsStorageManager
TRACE_LwipDnsResolver
TRACE_MultipartParser
TRACE_Network
TRACE_Router
TRACE_Tcp
TRACE_utility
TRACE_TimeManager
```

---

## 📓 Example Usage

To enable SD logging with timestamps and HTTP server tracing:

```cpp
#define TRACE_USE_SD 1
#define TRACE_USE_TIMESTAMP 1
#define TRACE_HttpServer 1
```

---

## 🧩 Advanced Setup

To trace your own module:

```cpp
#define TRACE_MyModule 1
#define TRACE_TAG MyModule
#include "framework_config.h"
#include "DebugTrace.h"

TRACE("This is a debug message\n");
```

---

For more examples, refer to the `/examples` directory and main documentation.
