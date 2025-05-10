# Ping-Pong Demo

This example demonstrates a pair of Pico devices communicating over HTTP using the PicoFramework. Each device ("ping-a" and "ping-b") alternately sends `/ping` requests to the other, and responds with `/pong`. The interaction is visible through the web interface and logs, illustrating real-time communication between two nodes.

---

## What You'll See

- A browser-based UI hosted from each device (served via LittleFS)
- A pair of controllers sending and responding to pings
- Clear log messages showing `/ping` and `/pong` HTTP exchanges
- A simple, event-driven design using FreeRTOS and PicoFramework

---

## What It Demonstrates

This example showcases:

- Bidirectional HTTP communication between embedded devices
- Use of `HttpClient` and `HttpServer` within the same application
- Serving static HTML from flash via LittleFS
- Pre-loading a littlefs image based on the /html directory
- Compile-time configuration using `THISHOST`/`THATHOST` macros
- Simple controller logic built with `FrameworkController`
- Fully FreeRTOS-based concurrency
- Switching between USB and Picoprobe flashing

---

## Prerequisites

- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- FreeRTOS Kernel path
- `picotool` (for USB flashing) or `openocd` with PicoProbe
- LittleFS image tool: `mklittlefs` from PicoFramework tools

---

## Usage

### 1. Choose your host identity (ping-a or ping-b)

You must define which device this is at build time:

```bash
# Choose ping-a or ping-b
cmake -B build -DTHISHOST=ping-a .
```

This will automatically set `THATHOST` to the opposite role.

---

### 2. Build and Flash

Choose one of the following flashing methods:

#### 🔌 USB (BOOTSEL mode — requires holding BOOTSEL when plugging in):
```bash
cmake -B build -DFLASH_METHOD=usb -DTHISHOST=ping-a .
cmake --build build --target flash_all
```

#### 🧪 PicoProbe (OpenOCD, default):
```bash
cmake -B build -DFLASH_METHOD=picoprobe -DTHISHOST=ping-a .
cmake --build build --target flash_all
```

> Repeat with `THISHOST=ping-b` on the second device.

---

### 3. Clean Targets

If you're updating only the HTML UI:
```bash
cmake --build build --target clean_fs
```

To remove all binaries:
```bash
cmake --build build --target clean_all
```

---

## Notes

- The example uses LittleFS and requires a valid `memmap_lfs_fragment.ld` for your board.
- By default, `ping-a` sends the first request.
- Both devices must be on the same Wi-Fi network and have unique hostnames, your router must support mDNS to use names rather than ip addresses (most do).
- Logs are visible over UART or USB serial depending on board configuration.

---

## File Locations

- `src/` — Application and controller logic
- `html/` — Files served via LittleFS
- `build/` — Generated output (`.uf2`, `.img`, etc.)

---

## Credits

Built using [PicoFramework](https://github.com/pico-framework) with FreeRTOS and lwIP integration.
