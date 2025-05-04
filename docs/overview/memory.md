# RP2040 Framework RAM Summary (Detailed)

## 🧠 Static RAM (inside application)

| Component | Allocated inside app? | Size |
|:----------|:----------------------|:----:|
| FreeRTOS heap (configTOTAL_HEAP_SIZE) | ✅ Yes | 100KB |
| lwIP core stack buffers (MEM_SIZE, TCP buffers) | ✅ Yes | ~16–24KB |
| Framework and app static allocations | ✅ Yes | ~80–90KB |

➡️ All included inside the 208KB measured BSS size.

---

## 📦 Additional RAM Required (outside application)

| Component | Allocated outside app? | Size Estimate |
|:----------|:-----------------------|:-------------:|
| FreeRTOS kernel scheduler / TCBs / ISRs | ✅ Yes | 2–4KB |
| Pico SDK base drivers (UART, USB) | ✅ Yes | 6–10KB |
| Wi-Fi stack (only if Wi-Fi active) | ✅ Yes | 10–20KB |
| TLS runtime buffers (mbedTLS) | ❌ No (comes from FreeRTOS heap) | 0KB |
| lwIP stack buffers (TCP/UDP pools) | ❌ No (already allocated inside app) | 0KB |

---

## 🎯 Final Reserve Guidelines

| Scenario | Minimum Free RAM Needed |
|:---------|:------------------------:|
| No Wi-Fi | **8–14KB** |
| With Wi-Fi | **18–34KB** |

---

## 📋 Budget Example (RP2040)

| Field | Amount |
|:------|:------:|
| Total SRAM | 264KB |
| App + FreeRTOS heap BSS | 208KB |
| System reserve (w/ Wi-Fi) | 32KB |
| Final free user RAM | ~24KB |
