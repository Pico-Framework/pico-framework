
# PicoFramework Feature Checklist

This document provides an overview of implemented features, utilities, and system components in the PicoFramework.

---

## ✅ Core Architecture

- [x] Modular MVC-inspired structure
- [x] FrameworkApp base class for application entry
- [x] FrameworkController base class for route logic
- [x] FrameworkTask for FreeRTOS task abstraction
- [x] FrameworkManager for centralized initialization

---

## ✅ HTTP Server

- [x] Lightweight embedded HTTP server
- [x] Routing system with method/path matching
- [x] Middleware support (per-route, chainable)
- [x] Request & Response abstractions
- [x] Static file serving (SD/FatFs)
- [x] Multipart form parsing
- [x] MIME type detection

---

## ✅ Application Support

- [x] AppContext for global service access
- [x] Configuration of network, time, storage
- [x] JsonService for JSON file persistence
- [x] FrameworkModel for CRUD persistence
- [x] FrameworkView (light templating / HTML serving)

---

## ✅ Event System

- [x] Event struct with `type`, `payload`, and source
- [x] EventManager with publish/subscribe model
- [x] Event delivery using FreeRTOS task notifications
- [x] Task `onEvent()` support for receiving events

---

## ✅ TimerService

- [x] Schedule one-shot events by timestamp
- [x] Schedule recurring events by interval
- [x] Schedule daily events by time and day mask
- [x] Schedule start/stop events with duration
- [x] Cancel scheduled jobs by job ID
- [x] Built-in job ID management
- [x] Will retry missed jobs after reboot (planned)
- [ ] Persistence of scheduled jobs (planned for future)

---

## ✅ Time and RTC Support

- [x] TimeManager with NTP + RTC (DS3231)
- [x] PicoTime utility class for conversion/formatting
- [x] NTPClient with retry and DNS support
- [x] Compatible with both RP2040 and RP2040+RTC

---

## ✅ Storage Support

- [x] StorageManager interface
- [x] FatFsStorageManager implementation
- [x] File read, write, append, mkdir, exists, remove

---

## ✅ Logging

- [x] Logger class with:
  - [x] Console or SD card output
  - [x] Log levels (INFO, WARN, ERROR)
  - [x] Timestamped logs
- [ ] Log rotation (future)

---

## ✅ Debug Tracing

- [x] Lightweight macro-based tracing system
- [x] Per-module trace enablement
- [x] Trace level filtering
- [x] Optional timestamp in output
- [x] Output to SD or console
- [x] Configured via `framework_config.h`

---

## ✅ Utilities

- [x] URL parsing, decoding
- [x] MIME detection
- [x] TCP state & memory diagnostics
- [x] Runtime task stats
- [x] Heap info and PCB display
- [x] cppMemory allocator tracking
- [x] Idle memory measurement

---

## ✅ Testing (Planned)

- [ ] CppUTest integration
- [ ] Unit tests for Router, Controller, Request, etc.
- [ ] End-to-end route + HTTP tests

---

## ✅ Documentation

- [x] Full Doxygen comments for:
  - [x] All public headers
  - [x] All core classes
  - [x] Utility functions
  - [x] File-level doc blocks with author/license
- [ ] Auto-generated HTML/PDF docs (planned)

---

## ✅ Build / Environment

- [x] CMake-based build system
- [x] Modular file structure
- [x] Ready for Raspberry Pi Pico W

---

## ✅ Example App (In Progress)

- [x] Login endpoint
- [x] Token-based auth (JWT planned)
- [x] HTML frontend served from SD
- [x] GPIO control via Web UI
