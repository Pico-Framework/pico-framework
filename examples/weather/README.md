# Weather Example – PicoFramework

This example demonstrates how to build a simple weather dashboard using PicoFramework. It shows how to:

- Serve a responsive HTML UI embedded in firmware
- Automatically detect location via IP geolocation
- Fetch real-time weather data from the Open-Meteo API
- Provide weather data through a structured REST API
- Use JSON and dynamic route handling

---

## Features

- `GET /` serves the weather dashboard UI from `weather_html.h`
- `GET /api/v1/weather` returns current weather and 3-day forecast as JSON
- Auto-detects location using http://ip-api.com/json if lat/lon not specified
- Supports optional lat/lon query parameters
- Displays live temperature and forecast with icon mapping
- Includes a UI toggle between Celsius and Fahrenheit

---

## File Overview

| File | Purpose |
|------|---------|
| `App.cpp` | Main application, handles routing and weather logic |
| `main.cpp` | Starts the FreeRTOS scheduler and launches the app |
| `weather_html.h` | Embedded HTML UI for the weather dashboard |
| `weather.cpp/.h` | Parses and structures weather API responses |

---

## REST API

### `GET /api/v1/weather`

Returns JSON with the current temperature and a 3-day forecast.

**Optional query params:**

- `lat` – Latitude (float)
- `lon` – Longitude (float)

**Example Response:**

```json
{
  "location": "San Francisco, California",
  "current": {
    "temperature": 18.5,
    "description": "Current conditions",
    "icon": "wi-day-sunny",
    "date": "2025-05-18T13:00:00Z"
  },
  "forecast": [
    {
      "date": "2025-05-19",
      "high": 22.0,
      "low": 14.5,
      "description": "Partly Cloudy",
      "icon": "wi-day-cloudy"
    }
  ]
}
```

---

## UI Overview

The embedded dashboard shows:

- Current temperature
- Local time and description
- Icon-based 3-day forecast
- Unit toggle for °C/°F

Icons are from the [Weather Icons](https://erikflowers.github.io/weather-icons/) set.

---

## Running the Example

1. Flash your Pico device with this app
2. Connect to Wi-Fi and obtain an IP address
3. Open browser to `http://<device-ip>/` or `http://Pico-Framework` if your router has mDNS support

---

## 🧰 Notes

- Uses `HttpRequest` to fetch weather data and IP geolocation
- Includes fallback defaults if location detection fails
- Weather parsing is handled by a reusable `Weather` class
- UI is embedded directly in C++ via a raw string

---

## 📚 Further Reading

- [Open-Meteo API](https://open-meteo.com/)
- [IP-API for location lookup](http://ip-api.com/)
- [PicoFramework Documentation](https://picoframework.com)

