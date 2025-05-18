## PicoFramework Prelaunch Review and Action Plan

This document outlines the final priorities and recommendations for preparing PicoFramework for a soft release on GitHub. The goal is to ensure stability, polish, and professional presentation to a small initial audience (e.g. Raspberry Pi product stakeholders) by Wednesday.

---

### Demo Clean-Up \[Section 1]

#### Must-Do

* Remove `app.start()` from `main()` in all example apps
* Ensure all demos run correctly on both RP2040 and RP2040.2 (Pico 2)
* Ensure LittleFS and FatFS storage can be switched cleanly in each demo
* Remove hardcoded paths in examples (e.g., config and file paths)
* Validate `FatFs::format()` works and run full file manager demo
* Run a clean release build with all features enabled
* Validate build on a second machine — no hardcoded paths or toolchain assumptions

#### Lower Priority but Worth Checking

* Fix ping-pong demo fragment size (if relevant to demo clarity)
* Run memory usage checks (with and without optional modules)

---

### Documentation \[Section 2]

#### Must-Do

* Complete a clear and concise **Getting Started guide**
* Add READMEs to all demos:

  * What the demo does
  * What features it shows
  * How to build and run it
* Include and document `framework_config_user.h`
* For users without a debug probe, note that `picotool` must be used to flash filesystem image

#### Optional/Nice-to-Have

* Merge introduction and overview documents (only if time permits)
* Ensure examples have clean comments

---

### GitHub Cleanup \[Section 3]

#### Must-Do

* Remove unrelated or obsolete repos from public view (e.g., old `oddwires`, `ianarchbell` test repos)
* Add placeholder or minimal versions of:

  * `CONTRIBUTING.md`
  * `LICENSE.md`
  * `SECURITY.md`
  * Optional: PR or issue templates in `.github/`

#### Strongly Recommended

* Create a clear top-level `README.md` that includes:

  * Project purpose and supported hardware
  * 5–10 line getting started example
  * Feature summary
  * Link to docs and demos

---

### Testing \[Section 4]

#### Must-Do

* Fix any known broken unit or integration tests
* Confirm `Logger` trace-to-storage functionality works
* Confirm `HttpRequest::toFile()` works as designed (even if not demoed)

#### Can Wait (post-release)

* Add additional Mocha, CppUTest coverage
* Memory tracking, hard fault diagnostics, idle memory reporting

---

### Post-Testing Fixes \[Section 5]

Should not block soft launch unless regressions appear.

* Fix Safari/Chrome header issue (only if reproducible during demo)
* Fix `/api/v1/zones/%7Bid%7D` malformed route crash
* Investigate RTC/AON timing glitches in AON timer init
* Evaluate Wi-Fi restart error handling
* Validate client TLS connection behavior on non-RPi platforms
* Optional: Move `handle_static()` from router to request (architectural clarity)
* Fix missing underline/search box in Doxygen output

---

### Minor Omissions to Consider

* Add fallback embedded HTML view for demos if SD card is missing
* Add `CHANGELOG.md` stub to indicate version history (`v0.1.0-pre`) and intent

---

### Summary Guidance

| Area                | Priority | Action                                                              |
| ------------------- | -------- | ------------------------------------------------------------------- |
| Demo functionality  | 🔥       | Ensure all storage/filesystem demos work on both boards and configs |
| File manager demo   | 🔥       | Key showcase — must be flawless                                     |
| Getting Started     | 🔥       | Clear 5-minute guide with flashing notes and supported hardware     |
| GitHub presentation | 🔥       | Prune public repos, add README + meta files                         |
| Test surface        | 🟡       | Cover only what would visibly fail or hurt confidence               |
| Future work         | ⏳        | Leave OTA, WebSockets, Captive Portal for post-Wednesday milestone  |


