# File Storage Demo – PicoFramework

This demo showcases one of the most powerful features of PicoFramework: **a complete, extensible file storage system with built-in HTTP API and UI**. It allows developers to store, browse, upload, delete, and serve files directly from the device using either LittleFS or FatFs.

---

## Why It Matters

Modern microcontroller apps often need:

- Persistent user data (e.g., config, logs, uploads)
- File-based web assets (HTML, CSS, images)
- OTA firmware or media upload
- Storage abstraction (use SD card or internal flash interchangeably)

**PicoFramework solves this with a unified StorageManager abstraction**, backed by either:

- `LittleFsStorageManager` (on internal flash)
- `FatFsStorageManager` (on SD card)

This demo provides:

✅ A front-end UI  
✅ Full REST API for managing files  
✅ Multipart upload support  
✅ Safe formatting  
✅ Seamless integration via `AppContext`

---

## Features

| Feature | Description |
|--------|-------------|
| Web UI | Drag-and-drop file manager (HTML/JS in firmware) |
| List files | `GET /api/v1/ls/uploads` |
| Upload files | `POST /api/v1/upload` with `multipart/form-data` |
| Delete files | `DELETE /api/v1/files/uploads/foo.txt` |
| Format storage | `POST /api/v1/format_storage` |
| Serve files | `GET /uploads/foo.jpg` (or any path) |

---

## File Overview

| File | Description |
|------|-------------|
| `App.cpp` | Sets up the app and starts `StorageController` |
| `StorageController.cpp` | Handles all HTTP API routes |
| `FileStorage.cpp` | Wraps the `StorageManager` API |
| `FileStorage_html.h` | Embedded HTML front-end |
| `main.cpp` | Boots the app and launches FreeRTOS |

---

## REST API Details

### `GET /api/v1/ls/uploads`
Lists files in `/uploads`

### `POST /api/v1/upload`
Uploads a file via multipart form

### `DELETE /api/v1/files/uploads/<filename>`
Deletes the file from storage

### `POST /api/v1/format_storage`
Erases all stored files

### `GET /uploads/<filename>`
Serves a static file (e.g., an image or HTML page)

---

## 🗃️ How StorageManager Works

`StorageManager` is a runtime-selected abstraction, injected via `AppContext`. It provides:

- `open(path, mode)`
- `read`, `write`, `remove`, `listDirectory`, `formatStorage`
- Compatible with either **LittleFS** or **FatFs**

```cpp
auto storage = AppContext::get<StorageManager>();
storage->listDirectory("/uploads", files);
```

All APIs are consistent regardless of backend.

---

## 🖼️ Web UI

The UI is embedded in firmware via `FileStorage_html.h`. It provides:

- Current file list
- Upload button (styled)
- Format button (with confirmation)
- Path override
- Preview and delete

This makes it ideal for managing web assets on-device (e.g., SPAs, images, configs).

---

## 💡 Extending This Demo

You can easily:

- Integrate storage into your own app by calling `AppContext::get<StorageManager>()`
- Upload web files (e.g., HTML/CSS/JS) and serve them from `/`
- Add logging, JSON config, or persistent state

The storage system is designed to support real-world, production-grade use.

---

## 📚 See Also

- [LittleFS Documentation](https://github.com/littlefs-project/littlefs)
- [FatFs by ChaN](http://elm-chan.org/fsw/ff/00index_e.html)
- [Multipart Form Upload Spec](https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods/POST)

---

## 🌟 Summary

PicoFramework’s StorageManager is a major strength:
- Unified interface
- Flexible backends
- Web-based tools
- Embedded-system friendly
- Designed for real deployment use

This demo is just the beginning — start building persistent, file-based apps now.

