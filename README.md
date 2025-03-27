# PicoFramework: The Web-First Embedded Framework for the Raspberry Pi Pico

## Overview

**PicoFramework** brings a modern, expressive architecture to the world of embedded development. Inspired by Express.js and designed specifically for microcontrollers, PicoFramework makes it easy to build web-connected applications on the **Raspberry Pi Pico** and other FreeRTOS-based systems.

With a modular design, a full HTTP server, flexible routing, middleware, JWT authentication, and JSON storage services — all optimized for embedded constraints — PicoFramework offers a professional, scalable foundation for building robust IoT and control applications.

---

## Key Features

- 🕸 **Integrated HTTP Server**  
  Serve REST APIs or simple web UIs directly from the Pico, with a full-featured request/response lifecycle and persistent connection support.

- 🧭 **Flexible Express-style Router**  
  Define routes and HTTP methods with support for path parameters, query strings, middleware chaining, and route-specific logic.

- 🧱 **Middleware Architecture**  
  Use reusable middleware to handle authentication, validation, logging, and more. Middleware can be scoped globally or per-route.

- 🔐 **JWT Authentication (Optional)**  
  Built-in JSON Web Token parsing and verification, enabling secure, stateless auth without extra libraries.

- 📦 **JSON Service with Storage Abstraction**  
  Easily persist and retrieve configuration data using a built-in JSON serializer, backed by pluggable storage (e.g., SD card).

- 📁 **SD File Manager**  
  Read/write files, serve static assets, and manage configuration files with a simple abstraction over the SD card.

- 🔄 **Event Manager (NEW)**  
  Publish and subscribe to internal system events for decoupled module communication. Supports both task and ISR contexts.

- ⏰ **Scheduler**  
  Built-in task scheduling system for time-of-day events, interval-based jobs, or event-triggered actions — all tied to FreeRTOS.

- 🧠 **Object-Oriented Task Model**  
  Each controller, service, and client runs in its own task class. Tasks are lightweight, composable, and integrated with the event system.

- 🧩 **Dependency Injection (WIP)**  
  Simple service registration and injection system, designed for extensibility and testability even in constrained environments.

---

## Architecture at a Glance

PicoFramework is designed from the ground up to feel familiar to web developers, yet optimized for embedded environments:

[Diagram coming]


---

## Use Cases

- 🔧 **Embedded Control Panels** (e.g., sprinkler systems, lighting controllers, home automation)
- 📡 **IoT Gateways & Edge Devices**
- 📈 **Diagnostics Dashboards**
- 🔐 **Secure Configuration Interfaces**
- 🧪 **Lab Tools & Instrumentation**

---

## Getting Started

Visit the GitHub repository:  
👉 [https://github.com/ianarchbell/PicoFramework](https://github.com/ianarchbell/PicoFramework)

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
📩 [ian@picoframework.com](mailto:ian@picoframework.com)

---

**PicoFramework** brings structure, scalability, and modern design patterns to the embedded world.  

**Build more than firmware — build micro apps.**
