
# Persistent Storage in PicoFramework

PicoFramework provides a flexible and unified system for handling persistent data on embedded devices like the RP2040. This system is designed to support multiple types of storage hardware and use cases, ranging from serving static HTML files to storing configuration data and maintaining application state across reboots.

This document explains how storage works in the framework, why abstraction is used, and how to use each storage class. It includes complete examples and design rationale so you can confidently build apps with persistent storage.

---

## 1. Why Use an Abstract Storage Interface?

In embedded systems, there are multiple types of persistent storage:

- SD cards (removable, high capacity, FAT32)
- Internal flash (non-removable, low capacity, wear-limited)
- External SPI flash (sometimes)

PicoFramework introduces a unified interface called `StorageManager`, which hides the differences between these devices and lets application code interact with files in the same way, regardless of the backend.

### What This Enables:

- Applications can switch between SD card and internal flash by changing one line at startup.
- You can write tests against the abstract interface or mock storage entirely.
- Framework features like JSON persistence, static file serving, and upload handling don’t care which backend is used.

This level of decoupling is critical for both flexibility and testability in embedded environments.

---

## 2. Choosing Between LittleFS and FatFs

The framework supports two concrete implementations:

- **LittleFsStorageManager**: Uses the RP2040's internal flash memory.
- **FatFsStorageManager**: Uses an SD card via the SPI interface.

Each has strengths and tradeoffs.

### Use LittleFS when:

- You need reliable, small-footprint storage for config files or app state.
- You want guaranteed availability even without external hardware.
- Your data size is under 100KB or changes infrequently.

### Use FatFs when:

- You need to serve a website (HTML, CSS, JavaScript).
- You want to support file upload/download.
- You need to log large amounts of data over time.
- You want users to access files directly via card readers.

You can also build applications that **prefer SD** but fall back to **LittleFS** if the card is not present.

---

## 3. Overview of StorageManager API

The `StorageManager` interface includes everything you need for file I/O, directories, and file metadata. This includes:

- Mounting/unmounting the filesystem
- Reading and writing binary data
- Appending to files
- Listing and removing files or directories
- Checking existence and getting sizes
- Streaming large files in chunks

Example usage:

```cpp
auto* storage = AppContext::get<StorageManager>();

if (!storage->mount()) {
    printf("Storage not available\n");
    return;
}

std::vector<uint8_t> content = {'H','e','l','l','o'};
storage->writeFile("/hello.txt", content);

if (storage->exists("/hello.txt")) {
    printf("File written\n");
}
```

---

## 4. FatFsStorageManager: SD Card Storage

This implementation uses the FatFs library and SD cards connected via SPI. It is ideal for serving websites and handling uploaded files.

### Features:

- Filesystem: FAT32
- Compatibility: Cross-platform readable (macOS, Windows, Linux)
- Maps `/foo.txt` to `0:/foo.txt`
- Thread-safe with mutex
- Automatically creates missing directories

### Registering it:

```cpp
AppContext::register<StorageManager>(new FatFsStorageManager());
```

### File Listing Example:

```cpp
std::vector<FileInfo> files;
storage->listDirectory("/", files);
for (const auto& file : files) {
    printf("Name: %s, Size: %u\n", file.name.c_str(), file.size);
}
```

FatFs is especially useful for user-accessible content or where large file support is needed.

---

## 5. LittleFsStorageManager: Flash Storage

This backend uses part of the RP2040's internal flash, formatted with LittleFS.

### Features:

- Filesystem: LittleFS
- Mounts a defined flash region (default: last 128KB)
- No SD card needed
- Safe for small files that change occasionally
- Thread-safe with FreeRTOS mutex

### Registering it:

```cpp
AppContext::register<StorageManager>(new LittleFsStorageManager());
```

### Formatting Example:

```cpp
storage->formatStorage();  // Warning: wipes all files
```

LittleFS is the default choice for saving internal state and system-critical configuration.

---

## 6. JsonService: Key-Value JSON Storage

The `JsonService` provides higher-level access to structured JSON objects stored on top of a file. It is ideal for configuration, preferences, and status snapshots.

### Use Cases:

- App config (`/config.json`)
- Wi-Fi credentials or API keys
- Current device status
- Caching settings across reboots

### Setup:

```cpp
auto* storage = AppContext::get<StorageManager>();
AppContext::register<JsonService>(new JsonService(storage));
```

### Example:

```cpp
auto* json = AppContext::get<JsonService>();

if (!json->load("/config.json")) {
    printf("Creating default config\n");
    json->data()["device"] = "MyPico";
    json->save("/config.json");
} else {
    std::string name = json->data().value("device", "unknown");
    printf("Loaded config: %s\n", name.c_str());
}
```

This is ideal when you just need a simple key-value file without defining a model class.

---

## 7. FrameworkModel: Structured Object Arrays

FrameworkModel builds on `StorageManager` to provide persistent collections of structured objects. Each model is backed by a JSON file representing an array of items.

Example use cases:

- Zone lists
- Sprinkler programs
- Scheduled jobs
- User-defined rules

### Creating and Saving a Model:

```cpp
FrameworkModel programs(storage, "/programs.json");

programs.load();
programs.create({{"id", "p1"}, {"name", "Morning"}});
programs.save();
```

### Querying:

```cpp
auto obj = programs.find("p1");
if (obj.contains("name")) {
    printf("Program name: %s\n", obj["name"].get<std::string>().c_str());
}
```

FrameworkModel is ideal when your application works with sets of named records that should persist.

---

## 8. Streaming Large Files

For very large files (e.g., OTA images or long logs), you can stream content in chunks without allocating full buffers.

```cpp
storage->streamFile("/firmware.bin", [](const uint8_t* chunk, size_t len) {
    // Send to socket or hash in place
});
```

This is particularly useful for constrained memory systems.

---

## 9. Summary and Recommendations

| Layer             | Use When                                |
|------------------|------------------------------------------|
| StorageManager   | You need raw file access                 |
| JsonService       | You want a config or key-value store     |
| FrameworkModel    | You manage lists of objects like records |

By using these layers together, you can handle almost any persistent data need with confidence.

---

## Full Startup Example

```cpp
auto* fs = new LittleFsStorageManager();
AppContext::register<StorageManager>(fs);
AppContext::register<JsonService>(new JsonService(fs));

FrameworkModel zones(fs, "/zones.json");
zones.load();

if (zones.empty()) {
    zones.create({{"id", "z1"}, {"name", "Front"}});
    zones.save();
}
```

This example shows how everything ties together: storage registration, service layering, and model use.

