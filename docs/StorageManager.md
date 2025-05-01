# StorageManager – Abstract Storage Interface for PicoFramework

The `StorageManager` defines a generic, backend-agnostic interface for file and directory access. It is used across the framework (e.g., by `JsonService`, `HttpServer`, and `FileUploadHandler`) to support SD card or flash-based storage using interchangeable implementations like FatFs or LittleFS.

## ✅ Features

- Abstracts file/directory I/O with a unified interface
- Supports mounting, reading, writing, streaming, and formatting
- Backend-agnostic: usable with SD card (FatFs) or internal flash (LittleFS)
- Allows streaming files in memory-constrained systems
- Used throughout the framework for persistent storage and file serving

---

## 🔧 Usage Overview

### 1. Register your chosen implementation

Your chosen StorageManager will have been registered automatically in AppContext at startup time by the Framework.

FatFs (SD card):
```cpp
AppContext::register<StorageManager>(new FatFsStorageManager());
```

LittleFS (internal flash):
```cpp
AppContext::register<StorageManager>(new LittleFsStorageManager());
```

---

### 2. Access the storage service

```cpp
auto* storage = AppContext::get<StorageManager>();
```

---

## 🔹 Key Methods

### Mounting

```cpp
bool ok = storage->mount();
bool mounted = storage->isMounted();
```

---

### File Operations

```cpp
std::vector<uint8_t> buffer;
storage->readFile("/config/settings.json", buffer);
storage->writeFile("/log.txt", buffer);
```

Write from raw buffer:
```cpp
const uint8_t* data = ...;
size_t size = ...;
storage->writeFile("/raw.bin", data, size);
```

Append:
```cpp
storage->appendToFile("/log.txt", data, size);
```

---

### File Info

```cpp
bool exists = storage->exists("/path/file.txt");
size_t size = storage->getFileSize("/path/file.txt");
```

---

### Directories

```cpp
storage->createDirectory("/newdir");
storage->listDirectory("/", files); // `files` is a std::vector<FileInfo>
storage->removeDirectory("/oldstuff");
```

---

### Streaming

Efficient for large files:
```cpp
storage->streamFile("/bigfile.dat", [](const uint8_t* chunk, size_t len) {
    // Handle chunk
});
```

---

### Formatting

```cpp
bool formatted = storage->formatStorage();
```

> ⚠️ Use with caution: this will erase all data.

---

## 🧱 FileInfo Struct

Used by `listDirectory()`:

```cpp
struct FileInfo {
    std::string name;
    bool isDirectory;
    bool isReadOnly;
    size_t size;
};
```

---

## 🧰 Example: Simple Read + Append

```cpp
auto* storage = AppContext::get<StorageManager>();

std::vector<uint8_t> buffer;
if (storage->readFile("/log.txt", buffer)) {
    printf("Read %zu bytes\n", buffer.size());
}

const char* msg = "System booted\n";
storage->appendToFile("/log.txt", (const uint8_t*)msg, strlen(msg));
```

---

## 🚫 Limitations

- Not thread-safe unless the implementation explicitly supports it (e.g., LittleFS with locking, FatFs with locking).
- Both the implmentations provided here ARE thread-safe.
- Filesystem-specific errors are not surfaced in detail.
- Some methods (e.g., `formatStorage()`) may be no-ops in certain backends depending on the implmentation.

---

## 📎 Implementations

- [`FatFsStorageManager`](./FatFsStorageManager.md)
- [`LittleFsStorageManager`](./LittleFsStorageManager.md)

---

© 2025 Ian Archbell – Part of the PicoFramework  
MIT Licensed
