# Choosing Between Flash and SD Storage on the RP2040

Embedded applications on the Raspberry Pi Pico or Pico W often need to persist files — configuration, HTML, logs, uploads, and more. The RP2040 supports both **internal flash storage** and **external SD cards**, but each is suited to different types of data and workloads.

This guide outlines the strengths and trade-offs of each, and provides a simple recommendation for effective usage.

---

## ✅ Recommended Usage Pattern

Use internal flash (e.g. with **LittleFS**) for small, infrequently changed data.  
Use an SD card (e.g. with **FatFs**) for larger files, high-volume writes, or dynamic content.

### Use Flash (LittleFS) For:

| ✅ Use Case                              | 🧠 Why It's Ideal                      |
|------------------------------------------|----------------------------------------|
| Small configuration files                | Fast, always available, safe           |
| Credentials and tokens                   | Private and secure                     |
| System state flags (boot success, etc.)  | Compact, reliable                      |
| JSON-based preferences or metadata       | Efficient and structured               |
| Fast boot-time reads (<1 KB)             | Very low latency                       |

### Use SD Card (FatFs) For:

| ✅ Use Case                              | 🧠 Why It's Ideal                      |
|------------------------------------------|----------------------------------------|
| HTML, JavaScript, CSS assets             | Large, static files                    |
| File uploads or downloads                | Needs dynamic size                     |
| OTA firmware updates (binary files)      | Easily managed external media          |
| Logs or sensor data streams              | High-frequency, append-only            |
| Large assets (images, fonts, etc.)       | Avoids flash wear, higher bandwidth    |

---

## 🧱 Technical Background

### Flash Characteristics

- **Erase granularity**: 4 KB (fixed)
- **Write size**: 256 bytes (typical)
- **Requires erase before rewrite**
- **Limited endurance**: ~10,000–100,000 cycles per block
- **Subject to write amplification**:
  - Even a 1-byte write may trigger a 4 KB erase + rewrite
  - Slower and causes wear

### SD Card Characteristics

- **Managed flash**: Built-in controller handles wear leveling
- **Efficient block writes**: 512–4096 bytes
- **High capacity and speed**: Ideal for large or growing data
- **Removable**: Easy user access

---

## 🏁 Conclusion

For best results:

- Use **internal flash with LittleFS** for small, critical data that changes infrequently.
- Use an **SD card with FatFs** for large or user-facing data, such as websites, logs, uploads, and OTA files.

This strategy optimizes performance, reliability, and lifespan across both storage types.

---
