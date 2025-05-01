# LittleFsStorageManager – Internal Flash Filesystem for PicoFramework

`LittleFsStorageManager` implements `StorageManager` using [LittleFS](https://github.com/littlefs-project/littlefs) on the device's internal flash. It is ideal for applications that require persistent storage but no SD card.

## ✅ Features

- Uses internal flash with configurable offset and size
- Thread-safe with static FreeRTOS mutex
- Automatically mounts on first access
- Safe to use in multi-core and interrupt-driven environments
- Good for configs, credentials, uploaded assets

---

## 🔧 Setup

```cpp
AppContext::register<StorageManager>(new LittleFsStorageManager());
```

---

## 📦 Flash Layout

- Default base: `0x101E0000`
- Size: 128 KB
- Block size: 4 KB
- Configurable at build/link time if needed

---

## 🧰 Usage Example

```cpp
auto* storage = AppContext::get<StorageManager>();

std::vector<uint8_t> buffer;
if (storage->readFile("/config.json", buffer)) {
    printf("Read %zu bytes from flash\n", buffer.size());
}
```

See StorageManger for the full interface.

---

## 🛡️ Thread Safety

Internally uses `xSemaphoreCreateMutexStatic()` for locking. Safe for use across tasks and from ISRs when coordinated properly.

---

## ⚠️ Formatting

```cpp
storage->formatStorage();
```

Erases and reinitializes the LittleFS volume. Should be used only when corruption or reset is needed.

---

© 2025 Ian Archbell – Part of the PicoFramework  
MIT Licensed
