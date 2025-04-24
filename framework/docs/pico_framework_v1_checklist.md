
# PicoFramework – v1 Feature Checklist

## Core HTTP Functionality
- [x] `HttpRequest` with:
  - [x] Method, path, query, headers
  - [x] Body parsing (raw, form, JSON)
  - [x] Cookies (`getCookie()`)
  - [x] Client IP (`setClientIp()`)
- [x] `HttpResponse` with:
  - [x] Status, headers, and send
  - [x] JSON output (`res.json()`)
  - [x] Cookie handling
  - [x] Redirect
  - [x] File streaming (`sendFile()`)
  - [x] Chunked transfer support
- [x] Streaming support with backpressure delay (`STREAM_SEND_DELAY_MS`)
- [ ] `TcpConnectionSocket` abstraction (planned, not implemented)

## Routing & Middleware
- [x] Router with dynamic path matching (`/device/{id}`)
- [x] Global and per-route middleware support
- [x] Middleware short-circuit behavior
- [x] `loggingMiddleware` and `authMiddleware` samples
- [x] Route printing (`printRoutes()`)

## Authentication
- [x] JWT-based authentication via `JwtAuthenticator`
- [x] `authMiddleware` to enforce route protection
- [x] `/auth` route for token introspection

## File Uploads & Static File Serving
- [x] `MultipartParser` for `multipart/form-data` upload
- [x] Overwrite protection on file uploads
- [x] `HttpFileserver` for serving static files
- [x] SD card writing via `FatFsStorageManager`

## Utilities & Helpers
- [x] `JsonRequestHelper` for structured body access
- [x] `res.setHeader()`, `res.setCookie()`, `res.clearCookie()`
- [x] Middleware-friendly signatures and chaining
- [x] Debug logging + `TRACE()` macros
- [x] Cookie, content-type, method helpers

## Embedded Compatibility
- [x] FreeRTOS task-per-client handling with semaphore limit
- [x] Uses lwIP sockets (not raw API)
- [x] Minimal allocations
- [x] No exceptions
- [x] Easy to disable components (e.g., no auth, no file upload)

## 🟡 Optional / V2 Candidates
- [ ] `req.isContentType()`
- [ ] `res.headersSent`
- [ ] `res.getHeader()`
- [ ] Named `req.param("key")` instead of vector-only
- [ ] `req.isSecure()` / protocol
- [ ] View/template rendering
- [ ] Multiple file parts in upload
- [ ] Extended `Accept:` handling or content negotiation
