# JWT Authorization Example – PicoFramework

This example demonstrates a full authentication flow using **JSON Web Tokens (JWT)** — a widely used method of securing APIs and user sessions — within the PicoFramework.

It includes:

- User signup and login
- Secure token generation and validation using JWT
- Route protection with middleware
- Persistent user storage
- A responsive embedded login UI

This architecture is similar to models used in modern web apps (e.g., Node.js + Express + JWT), adapted for microcontroller environments.

---

## 🔐 What is JWT?

**JSON Web Token (JWT)** is a compact, URL-safe means of representing claims between two parties. A JWT is:

- Signed (to verify integrity)
- Encoded as a base64 string
- Included in HTTP headers

**JWT Format:**
```
<Header>.<Payload>.<Signature>
```

**Example Payload:**
```json
{
  "sub": "alice",
  "name": "alice",
  "iat": 1716000000
}
```

JWTs are commonly used for:

- Stateless authentication (no server-side session needed)
- API authorization

In this demo, JWTs are generated and verified entirely within the embedded device.

---

## 📦 Authentication Flow

The full lifecycle of authentication looks like this:

```text
[Client] --- POST /signup or /auth with username/password ---> [Server]
[Server] --- verifies credentials and returns JWT ---------> [Client]
[Client] --- stores token (e.g., localStorage) ------------> (Client storage)
[Client] --- GET /api/v1/protected-data with Authorization ---> [Server]
[Server] --- validates JWT and returns secure data --------> [Client]
```

---

## 📁 File Overview

| File | Role |
|------|------|
| `App.cpp` | Starts the application and server |
| `UserController.cpp` | Defines routes: signup, login, and protected API |
| `UserModel.cpp` | Stores/loads users from `users.json` |
| `UserView.cpp` | Renders the login HTML view |
| `JwtAuthenticator` (framework) | Generates and verifies JWT tokens |
| `authMiddleware` (framework) | Validates token on protected routes |

---

## 🌐 Endpoints

### 🔸 `POST /signup`

Creates a new user and issues a token.

**Request JSON:**
```json
{ "username": "alice", "password": "secret" }
```

**Response:**
```json
{ "token": "<jwt_token>" }
```

---

### 🔸 `POST /auth`

Authenticates existing users.

**Request JSON:**
```json
{ "username": "alice", "password": "secret" }
```

**Response:**
```json
{ "token": "<jwt_token>" }
```

---

### 🔸 `GET /api/v1/protected-data`

Protected route. Requires an `Authorization` header.

**Request Header:**
```
Authorization: Bearer <jwt_token>
```

**Response:**
```json
{ "message": "You are authorized!" }
```

---

## 🔒 How Middleware Works

Middleware in PicoFramework is used to **intercept** and **process** requests before they reach the handler.

### `authMiddleware`

This is a function that:

1. Extracts the `Authorization` header.
2. Parses the `Bearer <token>` string.
3. Verifies the token signature and claims using `JwtAuthenticator`.
4. Rejects the request (`401 Unauthorized`) if the token is missing or invalid.

```cpp
router.addRoute("GET", "/api/v1/protected-data",
    handler,         // Your handler logic
    {authMiddleware} // Middleware to run first
);
```

---

## 🧠 Where Data Is Stored

- Users are stored in `users.json` via `UserModel`, which uses `FrameworkModel`.
- Tokens are never stored on the device — they are **stateless** and verified on each request.
- JWT signing uses a static secret key inside `JwtAuthenticator` (may be extended to use secure keys or expiry limits).

---

## 💡 Design Notes

- This pattern is **very similar** to what Node.js, Express, and frontend SPAs (like React or Vue) use.
- Clients store the token and attach it to every request to prove identity.
- The server checks it using the signing secret — no need for a login session or server-side state.

---

## ✅ Running the Demo

1. Flash the example to your device.
2. Access `/login` via browser.
3. Use DevTools or a fetch script to:
   - Call `/signup` or `/auth` with credentials
   - Store the token
   - Call `/api/v1/protected-data` using `Authorization` header

---

## 📚 Further Reading

- [JWT Introduction](https://jwt.io/introduction/)
- [RFC 7519 – JWT Specification](https://datatracker.ietf.org/doc/html/rfc7519)
- [PicoFramework Documentation](https://picoframework.com)

