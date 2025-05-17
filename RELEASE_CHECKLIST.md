# PicoFramework Final Cleanup and Release Checklist

This document outlines all remaining work required to finalize PicoFramework for public release. No new features will be added — focus is now on bug fixes, testing, documentation, and installation consistency across systems.

---

## 1. Bug Fixes

- [ ] Fix "Safari" or Chrome no-headers issue on initial connection
- [ ] Finalize AON timer handling (revisit rtc_init and compatibility)
- [ ] Ensure Wi-Fi restart events are reliably trapped and logged
- [ ] Remove all hardcoded paths in examples and framework
- [ ] Remove `app->start()` from `main.cpp` in example apps (now managed by FrameworkManager)
- [ ] Confirm fragment size and response chunk handling are consistent
- [ ] Handle `/api/v1/zones/%7Bid%7D` malformed route safely (avoid crash)
- [ ] Consider moving `handle_static_request()` from `Router` to `HttpRequest` for clarity

---

## 2. Documentation

- [ ] Write "Getting Started" guide (for first-time users and flashing)
- [x] Generate README for sprinkler demo (complete)
- [ ] Generate READMEs for all other demos
- [ ] Review and standardize all in-source example comments
- [ ] Review all HTTP API handler usage for correctness and completeness
- [ ] Merge "Intro" and "Overview" documents for clarity
- [ ] Fix Doxygen theme bugs: underline highlight, missing search box
- [ ] Add note for users without a Picoprobe: how to flash LittleFS using `picotool`

---

## 3. Testing and Validation

- [ ] Validate that all examples build on a clean second machine
- [ ] Remove hardcoded build paths and ensure portable builds
- [ ] Confirm TLS client support is usable on all platforms, or document Raspberry Pi-only restriction
- [ ] Implement and test `HttpRequest::toFile()` (has not been used yet)
- [ ] Confirm `TRACE()` macros for user code work (check `framework_config.h` usage in app)
- [ ] Add test for trace-to-storage output
- [ ] Fully test FatFs format, file I/O, and directory listing using FileManager demo
- [ ] Test release builds with and without optional modules (FatFs, TLS, auth)
- [ ] Re-enable and fix any broken CppUTest unit tests
- [ ] Add more unit tests for: Scheduler, EventManager, StorageManager
- [ ] Add more end-to-end API tests (e.g., with Mocha)
- [ ] Test idle memory reporting, HardFault handler, and memory diagnostics
- [ ] Confirm all demos that use storage (e.g., file upload) switch cleanly between LittleFS and FatFs

---

## 4. Build System and Portability

- [ ] Standardize CMakeLists.txt files across framework and examples
- [ ] Ensure `.ld` memory region configurations are correct:
- [ ] Sprinkler demo currently uses 256K — align others appropriately
- [ ] Avoid board-specific memory assumptions unless required
- [ ] Ensure storage offsets and `mkfs_littlefs` integration work consistently
- [ ] Verify `mklittlefs` and generated images flash correctly for all examples

---

## Final Steps

- [ ] Validate everything on second device from a clean clone
- [ ] Perform final doc pass and tag v0.1.0
- [ ] Push public documentation to GitHub Pages
- [ ] Optional: create example walkthrough video or screenshots

---

MIT © 2025 Ian Archbell
