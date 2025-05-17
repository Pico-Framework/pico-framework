# Sprinkler Controller Example (PicoFramework)

This example demonstrates a complete, event-driven sprinkler controller built with [PicoFramework](https://github.com/picoframework/pico_framework). It combines scheduled zone control, web-based configuration, structured logging, and real-time feedback into a cohesive, testable embedded application.

---

## 🌱 What This Demo Does

- Controls multiple irrigation zones via GPIO
- Supports program scheduling by day and time
- Uses an HTTP API and web interface for configuration
- Stores configuration and logs persistently (via LittleFS or FAT)
- Displays live and historical zone activity
- Demonstrates structured event-driven design

---

## 🧩 Framework Features Used

This example exercises a broad set of PicoFramework components:

| Component | Purpose |
|----------|---------|
| **HttpServer** | Serves API and frontend assets (HTML, CSS, JS) |
| **HttpRouter** | Maps routes to controller logic |
| **HttpRequest / HttpResponse** | Used in all route handlers |
| **FrameworkController** | SprinklerController handles zone actions |
| **FrameworkModel** | Program and Zone configuration stored in JSON |
| **TimerService** | Schedules start/stop zone events at specific times |
| **EventManager** | Publishes RunZoneStart, RunZoneCompleted, etc. |
| **Logger** | Outputs structured log messages with ISO UTC timestamps |
| **LittleFsStorageManager / FatFsStorageManager** | Persists program/zone config and logs |
| **JsonService** | Manages configuration JSON files via StorageManager |
| **AppContext** | Access to shared services (Logger, EventManager, etc.) |

---

## 📺 Frontend Interface

- **SPA (Single Page App)** using native Web Components
- Responsive layout
- Pages:
  - **Dashboard** — shows current zone status and last run times
  - **Programs** — create/edit/delete watering programs
  - **Zones** — view and name each zone (image upload supported)
  - **Log** — real-time activity log with auto-refresh

---

## 🚀 How to Use It

### 1. Build and Flash

Choose one of the following flashing methods:

#### USB (BOOTSEL mode — requires holding BOOTSEL when plugging in):
```bash
cmake -B build -DFLASH_METHOD=usb  .
cmake --build build --target flash_all
```

#### PicoProbe (OpenOCD, default):
```bash
cmake -B build -DFLASH_METHOD=picoprobe .
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


### 🌐 Connect to Wi-Fi

The app automatically connects to Wi-Fi and starts the HTTP server.

Access it in your browser at:

```
http://pico-framework/
```

Or by IP address shown in the serial console.

---

### 🧪 API Routes

| Method | Path | Description |
|--------|------|-------------|
| `GET` | `/api/v1/programs` | List all programs |
| `GET` | `/api/v1/zones` | List zones |
| `POST` | `/api/v1/programs` | Add new program |
| `PUT` | `/api/v1/programs/{name}` | Update program |
| `DELETE` | `/api/v1/programs/{name}` | Remove program |
| `POST` | `/api/v1/zones/{name}/start` | Start a zone manually |
| `POST` | `/api/v1/zones/{name}/stop` | Stop a zone manually |
| `GET` | `/api/v1/next-schedule` | Show next scheduled run |
| `GET` | `/api/v1/logs/summary` | Get structured log text |
| `GET` | `/api/v1/logs/summaryJson` | Get parsed zone run log |

---

## 🧠 Architectural Notes

- Programs are stored in JSON (`programs.json`), with fields:
  ```json
  {
    "name": "Morning Watering",
    "start": "06:30",
    "days": 73,
    "zones": [
      { "zone": "Front Lawn", "duration": 300 },
      { "zone": "Back Garden", "duration": 480 }
    ]
  }
  ```

- Programs are scheduled using `TimerService::scheduleDailyAt()` and trigger events (like `RunZoneStart`) published via `EventManager`.

- Each zone runs one at a time, in sequence, using posted `RunZoneCompleted` events to trigger the next.

- All time values are stored in **UTC**, and the frontend converts them to **local time** using JavaScript utilities.

---

## 📂 File Layout (Key Parts)

```
examples/sprinkler/
├── src/
│   ├── SprinklerController.cpp/.h   # Handles HTTP and zone control logic
│   ├── SprinklerScheduler.cpp/.h    # Triggers zone actions from TimerService
│   ├── ProgramModel.cpp/.h          # Loads/saves program config
│   └── ZoneModel.cpp/.h             # Controls GPIO pin per zone
├── assets/                          # Frontend HTML, JS, CSS
├── build/                           # Compiled firmware output
├── CMakeLists.txt                   # Build definition
```

---

## 🧰 Storage

- **Zone and program data**: stored in LittleFS (`/programs.json`, `/zones.json`)
- **Log file**: stored as `log.txt`
- The framework automatically initializes and formats storage as needed

---

## ✅ What You’ll Learn

- How to write a production-grade HTTP API on microcontrollers
- How to use event-driven task scheduling for GPIO control
- How to build and test a real frontend with zero dependencies
- How to structure persistent configuration using JSON
- How to safely store logs and configuration in flash or SD

---


---

## 📃 License

MIT © 2025 Ian Archbell
