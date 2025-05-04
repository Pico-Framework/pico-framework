
# Network Class - Wi-Fi Setup and Resilience

## Purpose

The `Network` class provides a static utility to start and manage Wi-Fi connections on Pico W using the CYW43 driver. It abstracts the low-level initialization, retry logic, and resilience strategies.

## Responsibilities

- Initialize the CYW43 driver
- Enable station mode
- Attempt connection with retries
- Poll for link status and IP acquisition
- Support re-connection if link drops
- Allow full restart of the Wi-Fi stack if needed

## Usage

Typical call from the framework:

```cpp
bool success = Network::startWifiWithResilience();
```

To check and reconnect if disconnected:

```cpp
if (!Network::checkAndReconnect()) {
    // Try to handle gracefully or schedule restart
}
```

To force a restart:

```cpp
Network::restart_wifi();
```

## Notes

- Wi-Fi credentials are expected via environment (`WIFI_SSID` and `WIFI_PASSWORD`)
- Link status is polled every second during connection
- LED is used to indicate connection state during attempts
- Power management is temporarily disabled during connection to ensure stability

## Link Status Values

- `CYW43_LINK_UP`: connected with IP address
- `CYW43_LINK_NOIP`: connected but waiting for DHCP
- `CYW43_LINK_JOIN`: associating with AP
- `CYW43_LINK_DOWN`: not connected

## Design Notes

- Framework does not retry indefinitely; it respects `WIFI_MAX_RETRIES`
- Intended for early boot and resilience, not runtime recovery logic (which user can layer on top)


## Debugging and Diagnostics

To help with debugging Wi-Fi connection issues, the class prints diagnostic output to the console during:

- Initialization
- Connection attempts and retries
- Link status polling
- IP acquisition (DHCP)

Example console output:

```
[Network] Connecting to WiFi SSID: my-network (attempt 1)
[Network] Joining network...
[Network] Acquiring IP address...
[Network] Connected to Wi-Fi network at 192.168.1.42
```

You can redirect or filter these logs using your own tracing or logging macros.

## Runtime Resilience Strategies

While `Network` handles boot-time resilience, users may want to monitor and recover from mid-session Wi-Fi dropouts:

```cpp
void monitorConnection() {
    if (!Network::isConnected()) {
        if (!Network::checkAndReconnect()) {
            printf("[App] Unable to reconnect. Will retry later.\n");
        }
    }
}
```

You can schedule this check using a `FrameworkTask` with `runEvery()` or similar polling logic.

## Known Limitations

- WPA2-Personal (PSK) only. Enterprise Wi-Fi is not supported.
- No automatic reconnection timer — must be scheduled by the application.
- Static IP is not supported in the current implementation.
- Cannot operate in Access Point (AP) mode.
- Requires correct `WIFI_SSID` and `WIFI_PASSWORD` set at build or runtime.

## Power Management Notes

The CYW43 chip supports power-saving modes. By default, power saving is re-enabled after connection:

```cpp
cyw43_wifi_pm(&cyw43_state, CYW43_DEFAULT_PM);
```

Power saving is temporarily disabled during connection attempts to improve reliability. You can disable power saving permanently if needed, though this will increase power consumption.

## Summary

Use the `Network` class to:

- Set up Wi-Fi safely at startup
- Recover from occasional disconnections
- Integrate your own logic around retry, logging, or fallback

Avoid using it for:

- Full runtime reconnection management (handle via tasks)
- Custom Wi-Fi modes (AP, SoftAP) — not currently supported

