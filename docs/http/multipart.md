## Multipart File Uploads

File uploads are an essential capability in embedded web servers — enabling users to upload firmware, configuration files, media assets, or logs directly through the web interface. Pico-Framework provides built-in support for multipart file uploads using a streaming parser designed for memory efficiency.

This section covers:

- Multipart form uploads and their structure
- Memory-safe streaming to disk
- Configuring the upload directory
- Handling metadata and form fields
- Error handling and large file protection
- Practical use cases

---

### Upload Route Setup

To handle uploads, define a route that matches the incoming `POST` request. The framework automatically detects multipart content via the `Content-Type: multipart/form-data` header and invokes the `MultipartParser`. Uploaded files are stored in an /uploads directory on either flash or SD card depending on the StorageManager you are using. The default can be changed in framework_config.h.

```cpp
router.post("/api/v1/upload", [this](HttpRequest& req, HttpResponse& res) {
    return this->handleUpload(req, res);
});
```

The body is parsed and stored to disk as the data arrives — a single 1460 byet buffer is used and each buffer is appended the file enasuring that memory will not limit the upload.

### Uploading via Browser or curl

HTML form example:

```html
<form action="/api/v1/upload" method="post" enctype="multipart/form-data">
  <input type="file" name="file">
  <input type="submit" value="Upload">
</form>
```

curl example:

```bash
curl -F "file=@firmware.bin" http://device.local/api/v1/upload
```

### Multiple File Fields

The current parser supports one file field per upload request. If multiple files are needed, upload them individually, or implement multipart array parsing at the application level.

**[Example endpoint here]**

Summary
Pico-Framework provides a clean, memory-safe approach to handling file uploads:

* Streams file data directly to disk
* Supports standard browser and tool uploads
* Protects RAM usage and handles large files
* Automatically parses multipart/form-data and extracts metadata
  
It enables embedded devices to receive firmware, images, or configuration data without requiring external servers or complex logic.