# FatFsStorageManager – SD Card File Access for PicoFramework

`FatFsStorageManager` is the StorageManager implementation for SD cards using the FatFs + FreeRTOS-compatible API. It provides robust file and directory access and is ideal for high-capacity external storage.

## ✅ Features

- Uses standard FAT32 format for cross-compatibility
- Supports file and directory operations
- Mounts as `sd0` by default
- Thread-safe with optional FreeRTOS mutex protection
- Ideal for data logging, upload/download files, and backups

---

## 🔧 Setup

```cpp
AppContext::register<StorageManager>(new FatFsStorageManager());
```

> Make sure your SD card is wired and the SPI interface is initialized before mounting.

---

## 🧰 Usage Example

```cpp
auto* storage = AppContext::get<StorageManager>();

if (storage->mount()) {
    std::vector<uint8_t> data = {'H','e','l','l','o','\n'};
    storage->writeFile("/log.txt", data);
}
```
See StorageManager for the full interface.
---

## 🧱 Path Handling

Paths are automatically resolved relative to `sd0`, e.g. `/log.txt` resolves to `0:/log.txt`.

---

## 📎 Notes

- Only empty directories can be removed.
- `formatStorage()` will erase all files on the SD card.
- `streamFile()` reads in chunks, reducing memory usage.

---

© 2025 Ian Archbell – Part of the PicoFramework  
MIT Licensed
