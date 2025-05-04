
# TimeManager in PicoFramework

TimeManager provides a centralized and consistent way to manage system time, synchronize with NTP, detect timezones automatically, and convert timestamps into formatted strings with timezone awareness. It is a foundational service for any real-time or time-sensitive applications using PicoFramework.

This guide explains the purpose, architecture, and usage of TimeManager with real examples and design rationale.

---

## 1. Why Use TimeManager?

Time in embedded systems is notoriously error-prone:

- RTC clocks may drift or lose power
- Network clocks (NTP) are asynchronous and may fail
- Timezones are rarely handled correctly
- Timestamps are hard to make human-readable

TimeManager solves these problems by:

- Synchronizing time via SNTP (NTP over UDP)
- Setting the RP2040 Always-On Timer (AON)
- Optionally initializing the hardware RTC (if present)
- Formatting time with timezone labels
- Automatically detecting location-based timezone offsets using Open-Meteo

It provides a single service your app can rely on for accurate, human-readable timestamps.

---

## 2. Core Capabilities

- Start and sync time using SNTP
- Validate system time (sanity checks)
- Broadcast events when time becomes valid or is synced
- Support for timezone-aware formatting
- Use Open-Meteo and IP-based geolocation to detect the timezone offset
- Initialize AON or RTC time depending on platform
- Emit events like `SystemNotification::TimeSync`, `TimeValid`, and `TimeInvalid`

---

## 3. Initialization and Integration

TimeManager is automatically used by the framework and registered into `AppContext` at startup. It responds to two key lifecycle events:

- `onNetworkReady()`: Called once Wi-Fi is up. It starts the SNTP client and syncs time.
- `onHttpServerStarted()`: Called once HTTP server is live. It triggers timezone detection and offset application.

---

## 4. Example: Startup Flow

```cpp
TimeManager* tm = AppContext::get<TimeManager>();

// Check if time is valid (AON or RTC already running)
if (!tm->isTimeValid()) {
    tm->syncTimeWithNtp(10); // Wait up to 10 seconds
}

printf("Time: %s
", tm->currentTimeForTrace().c_str());
```

---

## 5. NTP Synchronization

TimeManager uses lwIP’s SNTP client. When a response arrives from the NTP server, the framework sets the AON timer using `sntp_set_system_time`.

Example output:

```
[TimeManager] Setting system time from epoch: 1714754891
[TimeManager] System time set to: Tue May 3 18:48:11 2025
```

Sanity checks ensure that bogus responses (e.g., year 1970) are rejected.

---

## 6. Timezone Detection and Offset

After startup, TimeManager uses `http://ip-api.com/json` to get a rough geolocation based on your public IP. It then queries the Open-Meteo forecast API to get the UTC offset in seconds and apply it.

```cpp
tm->detectAndApplyTimezone();
```

You can also manually apply a known offset:

```cpp
tm->applyFixedTimezoneOffset(3600, "CET", "CEST");
```

This changes the formatted string output for logs and UIs.

---

## 7. Formatting Time for Display

You can request a formatted string of the current time including the local zone:

```cpp
std::string stamp = tm->currentTimeForTrace();
// Output: [18:48:11 CET]
```

---

## 8. Handling Failures and Fallback

If NTP fails, TimeManager:

- Falls back to checking the AON timer
- If no valid time source exists, posts `SystemNotification::TimeInvalid`
- If only partial time is available (e.g. from RTC), it may still proceed

Apps can subscribe to these events to respond accordingly.

```cpp
void MyController::onEvent(const Event& event) {
    if (event.notification.isSystem(SystemNotification::TimeSync)) {
        printf("We now have accurate time!
");
    }
}
```

---

## 9. RTC Support (RP2040)

If RTC hardware is available (e.g. RP2040), TimeManager can initialize it using:

```cpp
rtc_init();
```

But the primary time source is the Always-On Timer (AON) for stability and precision.

---

## 10. Summary

TimeManager simplifies time handling by:

- Abstracting away NTP and timer setup
- Supporting timezone-aware formatting
- Automatically syncing time after network availability
- Broadcasting relevant events to the system

It is a plug-and-play service that your controllers, schedulers, and logs can depend on.

---

## Common Use Cases

- Timestamping log messages
- Scheduling events based on local time
- Displaying time in a human-readable way in web UIs
- Adjusting behavior based on time-of-day

