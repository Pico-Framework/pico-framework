# Sprinkler Demo


---

## What You'll See


---

## What It Demonstrates



---

## Prerequisites

- [Pico SDK](https://github.com/raspberrypi/pico-sdk)
- FreeRTOS Kernel path
- `picotool` (for USB flashing) or `openocd` with PicoProbe
- LittleFS image tool: `mklittlefs` from PicoFramework tools

---

## Usage


---

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

## Credits

Built using [PicoFramework](https://github.com/pico-framework) with FreeRTOS and lwIP integration.
