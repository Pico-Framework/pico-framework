# JsonService – Persistent JSON Utility for PicoFramework

The `JsonService` is a general-purpose helper class for working with JSON-based storage in your embedded applications. It allows you to load, modify, and save JSON files using any registered `StorageManager`.

## ✅ Features

- Load and save JSON files from persistent storage
- Access and modify the JSON structure directly
- Pretty-prints saved output for easy debugging
- Gracefully handles empty or invalid files
- Fully decoupled from model or controller logic
- Can be used for app settings, saved state, user data, etc.

---

## 🔧 Requirements

- A valid `StorageManager` (e.g., FatFsStorageManager or LittleFsStorageManager) must be registered in `AppContext` before using `JsonService`.

---

## 🔹 Usage

### 1. Get the instance from `AppContext`

```cpp
auto* json = AppContext::get<JsonService>();
```

> 💡 You must have registered it in `FrameworkManager` or your app initializer:
```cpp
AppContext::register<JsonService>(new JsonService(AppContext::get<StorageManager>()));
```

---

### 2. Load a JSON file

```cpp
if (!json->load("/config/settings.json")) {
    printf("Failed to load settings\n");
}
```

---

### 3. Read or write JSON content

```cpp
auto& data = json->data();

std::string name = data.value("device_name", "Unnamed");

data["device_name"] = "Sprinkler Controller";
data["timezone"] = "America/Los_Angeles";
```

---

### 4. Save changes

```cpp
if (!json->save("/config/settings.json")) {
    printf("Failed to save settings\n");
}
```

---

### 5. Optional: Access using operator*

```cpp
(*json)["enabled"] = true;
```

---

### 6. Optional: Merge with defaults

```cpp
nlohmann::json defaults = {
    {"device_name", "Unnamed"},
    {"enabled", true},
    {"threshold", 42}
};

auto& data = json->data();
data = mergeDefaults(data, defaults);
```

---

## 🧪 Validity Check

```cpp
if (!json->hasValidData()) {
    // Handle parse error or discard
}
```

---

## 🚫 Notes & Limitations

- Manages one JSON document per instance.
- Requires explicit `load()` and `save()`.
- Not thread-safe on its own.

---

## 🧰 Example: Storing Wi-Fi Credentials

```cpp
auto* json = AppContext::get<JsonService>();

if (json->load("/config/wifi.json")) {
    std::string ssid = json->data().value("ssid", "");
    std::string pass = json->data().value("password", "");

    if (ssid.empty()) {
        json->data()["ssid"] = "MyNetwork";
        json->data()["password"] = "secret123";
        json->save("/config/wifi.json");
    }
}
```

---

## 📎 Related

- [`StorageManager`](./StorageManager.md) – Required backend for JsonService
- [`PicoModel`](./PicoModel.md) – May use JsonService internally for state persistence

---

© 2025 Ian Archbell – Part of the PicoFramework  
MIT Licensed
