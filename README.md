# PicoFramework: The Web-First Embedded Framework for C++

## Overview

**PicoFramework** brings a modern, expressive architecture to the world of embedded development in C++. Inspired by Express.js and designed specifically for microcontrollers, PicoFramework makes it easy to build web-connected applications on the **Raspberry Pi Pico**. The first platform available is the Raspberry Pi Pico.

With a modular design, a full HTTP server, flexible routing, middleware, JWT authentication, and JSON storage services — all optimized for embedded constraints — PicoFramework offers a professional, scalable foundation for building robust IoT and control applications.

> [Full Documentation → https://Pico-Framework.com](https://Pico-Framework.com)

---

## Key Features

### Express-Style HTTP Routing
Define structured HTTP endpoints using method + path macros, with clean controller logic. Includes route variables, query parsing, file uploads, middleware, and automatic JSON encoding. You write simple, readable handlers that are easy to test and reason about.

```cpp
GET("/api/v1/zones", handleZones);

static void handleZones(HttpRequest& req, HttpResponse& res)
{
    res.json(getAllZones()); // Sends 200 OK + application/json
}
```

### JWT Authentication (Optional)
Built-in JSON Web Token (JWT) support enables secure authentication without needing extra tooling. Easily protect routes using middleware. Works with any frontend, see the authorization_route example.

### Unified Storage Layer
Abstract away the details of flash vs SD. Switch between LittleFS (internal flash) and FatFs (SD card) via a single `StorageManager` interface. Save binary files or structured JSON settings with `JsonService`.

### Persistent JSON Config
Store user preferences, schedules, or app state in structured files. Supports typed loading, atomic save, and directory listing. The `FrameworkModel` class provides easy record management.

### Time-of-Day Event Scheduler
Schedule events at fixed times or on repeating patterns. Define jobs like:
- `"Start zone 3 at 7:00am on Mon/Wed/Fri"`
- `"Stop after 10 minutes"`

Timers post events back to your controllers or use callbacks at a scheduled time.

### Event Bus + Notification System
The `EventManager` provides publish/subscribe event delivery. Events can be dispatched from tasks or ISRs, and consumed in controller-specific handlers. Avoid global state and deeply coupled logic.

### Static File Server + Upload
Serve HTML, CSS, JS, and images directly from flash or SD. Supports gzipped files and chunked transfer. Multipart upload support lets you update files live via browser.

### Built-in Test Framework
Run CppUTest-based unit tests on-device under FreeRTOS. Mocha is used for end-to-end testing.

---

## Real-World Ready

PicoFramework is a product-grade runtime for shipping connected systems. It's already been used for:

- Smart irrigation and zone control
- Interactive on-device dashboards
- Time-triggered automation and GPIO switching
- Local web configuration panels
- File uploads and logs over HTTP

You can write maintainable apps that combine web behavior, structured configuration, and tight control logic — without patching together libraries.

---

## 🧩 Included Example Apps

| App                | Highlights                                              |
|--------------------|---------------------------------------------------------|
| `minimum`          | One route - see how easy it is                          |
| `hello_framework`  | Minimal controller and route example - heavily commented                   |
| `route_authorization`        | Login + protected API using JWT               |
| `ping_pong`        | Device-to-device HTTP using TCP + TLS                   |
| `storage_manager`  | Manage files on flash or sd card                        |
| `sprinkler`        | Scheduler-driven GPIO zone activation w/ web UI         |
| `weather`        | Uses http client to get weather for local area        |
| `test_app`         | Pico Dashboard built with the framework         |

All examples build cleanly under FreeRTOS and use the same controller/View pattern as your app will.

---

## 📦 Out-of-the-Box Capabilities

| Feature                            | Status |
|------------------------------------|--------|
| GET/POST/PUT/DELETE routing        | ✅     |
| Path and query parameter parsing   | ✅     |
| JSON request and response support  | ✅     |
| Multipart/form-data upload         | ✅     |
| Static file serving from flash/SD  | ✅     |
| JWT-based authentication           | ✅     |
| TLS client support (mbedTLS)       | ✅     |
| Pub/sub event system (task/ISR-safe)     | ✅     |
| Daily job scheduling               | ✅     |
| Persistent structured config       | ✅     |
| Flash + SD abstraction             | ✅     |
| Embedded test runner               | ✅     |

---

## Design Philosophy

PicoFramework follows a clear and minimal philosophy:

- **Build once, run reliably** — Everything in the framework is designed for real-time embedded use. No surprises. No guesswork.
- **No tangled glue code** — The architecture gives you clear components: controller, view, model, service.
- **Test early and safely** — You can write actual unit tests that run on real hardware.
- **Event-driven logic, not polling loops** — Framework tasks wait on notifications, not spin or delay.
- **Use what you need** — Every feature is modular. Storage, auth, upload, file server — only link what you use.

---

## 🏗️ Getting Started

Clone the framework and build an example:

```bash
git clone https://github.com/Pico-Framework/pico-framework.git
cd Pico-Framework/examples/hello_framework
# Follow the README inside each example for specific build/setup instructions
```

You'll need:
- The Raspberry Pi Pico SDK
- FreeRTOS for RP2040 (already integrated)
- A supported toolchain (e.g. Arm GNU 14.2)

For full setup walkthroughs, see the docs:  
https://Pico-Framework.com/docs/getting-started

---

## 📚 Full Documentation

The full user guide, component reference, architecture notes, and examples are available online:

**https://Pico-Framework.com**

This includes:
- Getting started with flashing and SD cards
- Writing controllers and views
- StorageManager and JsonService
- Testing framework details
- Embedded file builds and upload handling
- Wi-Fi configuration and resilience logic
- Event system design

---

## Supported Boards

- RP2040 (e.g. Pico W)
- RP2350 (e.g. Raspberry Pi Pico 2 W, non-secure FreeRTOS mode)
- Compatible boards, although some features can be used without Wi-Fi. This is designed for connected boards.

Other ports (e.g., STM32, ESP32) are planned but not yet supported.

---

## 🔒 License

MIT License  
© Ian Archbell, 2024–2025

---

## 🤝 Contributing

We welcome issues, improvements, and architecture feedback. However, the first focus is ensuring this first early release meets the needs of the community so I am not planning large new feature releases. Everything will be incremental. This project is structured, maintained, and evolving with care. Please open a discussion before submitting major feature changes.

---

> Build confidently. Host dashboards. Automate reliably.  
> **All from the Raspberry Pi Pico.**


## Use Cases

- **Embedded Control Panels** (e.g., sprinkler systems, lighting controllers, home automation)
- **IoT Gateways & Edge Devices**
- **Diagnostics Dashboards**
- **Secure Configuration Interfaces**
- **Lab Tools & Instrumentation**

---

## Getting Started

Visit the GitHub repository:  
👉 [https://github.com/ianarchbell/Pico-Framework](https://github.com/pico-framework/pico-pframework)

or the full documentation at [https://Pico-Framework.com](https://Pico-Framework.com)
You'll find:
- Quick start guides
- Code examples
- Ready-to-run demo projects
- Documentation on tasks, routes, events, and configuration

---

## Licensing

PicoFramework is released under the **MIT License** to maximize adoption, contributions, and commercial flexibility.

---

## Contribute or Contact

Want to contribute or collaborate? Open an issue or pull request on GitHub, or email us at:  
📩 [ian@pico-framework.com](mailto:ian@pico-framework.com)

---

**PicoFramework** brings structure, scalability, and modern design patterns to the embedded world.  


