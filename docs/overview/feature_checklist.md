
# PicoFramework Feature Checklist

This document outlines the current features of PicoFramework as of the latest development state. All features are grouped by subsystem. Checkmarks (✓) indicate completed features, and future plans are noted where relevant. Designed to be used in HTML-rendered markdown without advanced bullet formatting.

---

## Core Architecture

✓ Modular MVC-inspired structure  
✓ `FrameworkApp` base class for main application  
✓ `FrameworkController` base class for logic and routes  
✓ `FrameworkTask` abstraction for FreeRTOS tasks  
✓ `FrameworkManager` for ordered startup and service registration  
✓ Application-defined `initRoutes()` and `poll()` support  
✓ Multicore-safe service access and task startup

---

## HTTP Server

✓ Lightweight HTTP server with request parsing  
✓ REST-style routing with method/path match  
✓ Route parameters and wildcards  
✓ Middleware support (authentication, logging, etc.)  
✓ `HttpRequest` and `HttpResponse` abstractions  
✓ Static file serving from SD or flash  
✓ Multipart file upload support  
✓ MIME type detection  
✓ Automatic fallback to embedded assets if file not found  
✓ Optional TLS support via socket wrapper

---

## HTTP Client

✓ `HttpClient` abstraction  
✓ Fluent API via `HttpRequest::setUri()`, headers, body, etc.  
✓ Full TLS with root certificate verification  
✓ Handles chunked responses and file writes  
✓ Designed for weather, cloud API, or OTA fetches  
✓ Uses `Tcp` abstraction to support multiple implementations

---

## Application Services

✓ `AppContext` for service registration and lookup  
✓ Structured `StorageManager` abstraction  
✓ `JsonService` for simple config persistence  
✓ `FrameworkModel` for object storage (arrays of items)  
✓ `FrameworkView` for templated HTML or JSON rendering  
✓ JWT authentication (middleware + route protection)  
✓ GPIO event handler and `GpioEventManager`  
✓ Embedded asset support for no-SD fallback

---

## Event System

✓ `Event` struct with type, payload, and source  
✓ `Notification` enum system with system/user distinction  
✓ `EventManager` with pub/sub task model  
✓ Per-task `onEvent()` delivery  
✓ Indexed task notifications using enums  
✓ ISR-safe event posting  
✓ Task event masks for filtering

---

## Timer and Scheduling

✓ `TimerService` for one-shot timers  
✓ Recurring interval events  
✓ Daily scheduling by time and weekday  
✓ Duration start/stop scheduling  
✓ Job cancellation by ID  
✓ Thread-safe job registry  
✓ Planned: Persistent jobs via JSON  
✓ Planned: Missed job replay after reboot

---

## Time and RTC Support

✓ `TimeManager` with SNTP sync  
✓ Time validity tracking and retries  
✓ AON/RTC setup on RP2040 and RP2350  
✓ Timezone detection via Open-Meteo  
✓ Human-readable formatting for timestamps  
✓ UTC/local conversion via `PicoTime`

---

## Storage Support

✓ Abstract `StorageManager` interface  
✓ `FatFsStorageManager` for SD card  
✓ `LittleFsStorageManager` for internal flash  
✓ File API: read, write, append, delete, mkdir  
✓ Thread-safe access  
✓ Safe formatting and fallback mounting  
✓ Optional file streaming (read/write in chunks)

---

## Logging

✓ `Logger` with INFO/WARN/ERROR levels  
✓ Timestamps in `YYYY-MM-DD HH:MM:SS` format  
✓ Console + file logging (SD or flash)  
✓ Log truncation warning  
✓ Manual log rotation logic  
✓ Web route integration for log viewing  
✓ Safe from multiple tasks

---

## Debug Tracing

✓ Lightweight macro-based trace system  
✓ Per-module enable/disable via flags  
✓ Optional timestamping  
✓ Output to console or file  
✓ Controlled via `framework_config.h`

---

## Utilities and Diagnostics

✓ MIME type mapping  
✓ URL parsing/encoding helpers  
✓ Task stats reporting  
✓ Heap/stack diagnostics  
✓ TCP memory diagnostics  
✓ Idle memory tracking  
✓ Memory-safe file uploads  
✓ Framework memory reporting planned for dashboard

---

## Testing and QA

✓ CppUTest integration  
✓ Embedded tests for core features  
✓ Route + HTTP end-to-end testing  
✓ File and model persistence tests  
✓ TLS + chunked stream test coverage  
✓ Mocha tests for web UI integration  
✓ Planned: Mock-based unit tests for client code

---

## Documentation

✓ Full Doxygen coverage (headers only)  
✓ Custom style and GitHub integration  
✓ HTML rendering with graphs and hierarchy  
✓ Markdown reference guides per subsystem  
✓ GitHub Pages support  
✓ Feature checklists and architecture overview

---

## Build System and Structure

✓ Modular CMake build for app and framework  
✓ Configurable build flags  
✓ Optional feature toggles (auth, storage, client)  
✓ Single-task or multi-task HTTP server modes  
✓ LittleFS flash linker support  
✓ Directory-mirrored header layout for organization

---

## Example App

✓ SD-backed website with login  
✓ JWT-authenticated routes  
✓ Zone list and sprinkler programs via JSON  
✓ Uploads and file browsing  
✓ GPIO control via API  
✓ Weather forecast integration  
✓ Fully portable between SD and flash  
✓ Interactive web dashboard and logs  
