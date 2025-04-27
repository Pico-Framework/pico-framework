# JsonResponse Helper Quick Guide

---

## ✨ Purpose

The `JsonResponse` module provides **simple, consistent** functions to send structured API responses:
- Success replies
- Error replies
- Created resources
- Empty success replies

✅ Keeps your API JSON responses **standardized** and **lightweight**.

---

## 🚀 Common Usage

### Sending a Success Response

```cpp
JsonResponse::sendSuccess(res, { {"id", 123} }, "Item created successfully");
```

Sends:

```json
{
  "success": true,
  "data": {
    "id": 123
  },
  "message": "Item created successfully"
}
```

---

### Sending a Created (HTTP 201) Response

```cpp
JsonResponse::sendCreated(res, { {"id", 456} }, "New resource created");
```

Sets status code **201 Created**.

---

### Sending a Plain Message

```cpp
JsonResponse::sendMessage(res, "Operation completed");
```

---

### Sending an Error Response

```cpp
JsonResponse::sendError(res, 404, "NOT_FOUND", "Item not found");
```

Sends:

```json
{
  "success": false,
  "error": {
    "code": "NOT_FOUND",
    "message": "Item not found"
  }
}
```

---

### Sending No Content (204)

```cpp
JsonResponse::sendNoContent(res);
```

Sends a clean **204 No Content** reply with no body.

---

## 📚 Full List of JsonResponse Functions

| Function | Description |
|:---------|:-------------|
| `sendSuccess(res, data, message)` | Send 200 OK with success payload |
| `sendCreated(res, data, message)` | Send 201 Created with success payload |
| `sendMessage(res, message)` | Send 200 OK with only a message |
| `sendNoContent(res)` | Send 204 No Content |
| `sendError(res, statusCode, code, message)` | Send error response with given code and message |

---

✅ **Tip:**  
You can also call these helpers **directly from `HttpResponse`** thanks to forwarding methods:

```cpp
res.sendSuccess({ {"key", "value"} }, "OK");
res.sendError(400, "BAD_REQUEST", "Invalid data");
```

---

# 🧐 Why Use JsonResponse?

- Consistent response structures
- Avoids repeated JSON formatting everywhere
- Cleanly separates success and error flows
- Keeps your route handlers simple and readable

---

✅ Done. Now your route handlers stay clean, predictable, and professional.

