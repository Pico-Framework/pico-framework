## Static File Serving and Gzip Support

One of the most powerful and user-friendly capabilities of the Pico-Framework HTTP server is its built-in static file serving. It allows you to host full web applications, dashboards, or configuration UIs directly from the device, even under constrained storage conditions. These assets can be served from multiple storage backends and support compression transparently.

This section covers:

- Static file serving architecture
- Supported storage options (SD card, LittleFS, Embedded)
- MIME type detection
- Gzip file detection and automatic Content-Encoding
- Fallback routing for single-page apps
- Real-world example scenarios

---

### Serving Static Files from Storage

By default, the framework maps URI paths to the virtual filesystem used by your application. A request like:

GET /index.html

Will resolve to:

- `/index.html` on the mounted SD card if using `FatFsStorageManager`
- `/index.html` on flash if using `LittleFsStorageManager`
- `index.html` from compiled-in assets if using `EmbeddedAssetManager`

This logic is implemented by `HttpFileserver`, which maps paths and calls `res.sendFile()` with appropriate headers.

#### Example Route for Static Content:

```cpp
router.get("/(.*)", [this](HttpRequest& req, HttpResponse& res) {
    return res.sendFile(req.uri());
});
```

This catch-all fallback ensures that any non-API route falls back to static content handling, which is ideal for SPAs and dashboards.

Storage Backends
The file server supports three interchangeable storage systems:

FatFsStorageManager
Uses an SD card via SPI or SDIO. Ideal for large apps or systems requiring removable storage.
LittleFsStorageManager
Mounts a LittleFS volume in flash memory. Persistent, fast, and available even without external hardware.
EmbeddedAssetManager
Serves files compiled directly into the firmware binary. Used as a fallback when others are unavailable.
These backends are selected via configuration and registered automatically by the framework at startup.

MIME Type Detection
The server uses built-in MIME type mapping to set the correct Content-Type headers based on the file extension:

**[this list needs updating]**
.html   → text/html
.css    → text/css
.js     → application/javascript
.png    → image/png
.svg    → image/svg+xml
.json   → application/json

This allows modern web apps to function correctly in browsers with no need for user configuration.

You can override or extend MIME types if needed by calling HttpFileserver::addMimeType().

### Automatic Gzip Compression Handling

The server supports serving pre-compressed .html, .css and .js files automatically generating the gzip Content-Encoding header. It determines whether to do this by checking the first two bytes of the file for the gzip "magic number"  (bytes 0x1f 0x8b).

```
Content-Encoding: gzip
```

This dramatically improves performance for larger pages and enables serving modern frontends like React or Vue apps.

### Single-Page Applications and Fallback Routing

For SPAs or dashboard-style UIs, you often want unknown routes to fall back to index.html. This can be done with:

```cpp
router.get("/(.*)", [this](HttpRequest& req, HttpResponse& res) {
    std::string path = req.uri();
    if (!res.sendFile(path)) {
        return res.sendFile("/index.html");
    }
    return true;
});
```
This allows client-side routers (e.g. Vue Router, React Router) to handle paths like /dashboard/settings, while the embedded server still serves the correct base file.

Note that regex expressions are precompiled so we can avoid exceptions at reuntime.

### Example: Full Static Content Setup

```cpp
void MyController::initRoutes() {
    // Serve API endpoints first
    router.get("/api/v1/device", [this](HttpRequest& req, HttpResponse& res) {
        return this->getDeviceInfo(req, res);
    });

    // Then fallback to file server
    router.get("/(.*)", [this](HttpRequest& req, HttpResponse& res) {
        if (!res.sendFile(req.uri())) {
            return res.sendFile("/index.html");
        }
        return true;
    });
}
```

This pattern ensures robust support for dynamic APIs and modern browser-based interfaces.

### Summary

The static file serving system in Pico-Framework enables full-featured web UIs on microcontrollers. With support for gzip compression, MIME types, flexible storage backends, and SPA fallback behavior, you can deploy everything from config pages to rich JavaScript dashboards with minimal effort and no custom code.