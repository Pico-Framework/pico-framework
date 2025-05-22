# Minimal Routing Example — PicoFramework

This example demonstrates the **smallest possible PicoFramework application** with a working HTTP server and a single `GET` route. It’s intended to show the **overall structure** of a real application without overwhelming you with detail.

If you're looking for a **fully explained version** of routing with extensive comments and usage patterns, see the [`hello_framework`](../hello_framework) example.

---

## 🧠 What It Shows

- How to define an application class (`App`) that inherits from `FrameworkApp`
- How to register a simple route using `router.addRoute(...)`
- How to start the HTTP server once the network is ready
- How to use `poll()` with `runEvery(...)` for simple periodic tasks
- How to subscribe to system events like `NetworkReady` or `TimeValid`

---

## 📡 Route Demonstrated

- `GET /`  
  Returns a plain text message and prints request headers to the console.

Example request:
```bash
curl http://<device-ip>/
```

---

## 🔧 Building

Make sure you've configured your environment, then run:
```bash
cmake -B build
cmake --build build
```

Then flash to your device using `picoprobe`, `openocd`, or UF2, depending on your setup.

---

## 🔁 Next Steps

Once you've run this, explore the [hello_framework](../hello_framework) example to see:

- Route parameters (`/user/{id}`)
- POST, PUT, DELETE handling
- Form and JSON body parsing
- Header inspection
- Use of `AppContext` and `EventManager`
