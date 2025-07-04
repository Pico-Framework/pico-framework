const char *weather_html = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Weather Dashboard</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/weather-icons/2.0.10/css/weather-icons.min.css">
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background: #e0e5ec;
            margin: 0;
            padding: 20px;
            display: flex;
            min-width: 220px; /* Ensure it doesn't get too narrow */
            flex-direction: column;
            align-items: center;
        }
        .card {
            display: grid;
            grid-row-gap: 10px;
            grid-template-rows: auto auto 1fr auto;
            grid-template-columns: 1fr;
            background: #e0e5ec;
            border-radius: 20px;
            box-shadow: 9px 9px 16px #bec3c9, -9px -9px 16px #ffffff;
            padding: 20px;
            min-width: 220px;
            max-width: 900px;
            width: 100%;
            margin-bottom: 20px;
            text-align: center;
            min-height: 140px;
        }
        .card h2 {
            margin: 0 0 10px;
        }

        .forecast {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 20px;
            margin: 0 auto;
            padding: 10px;
            min-width: 220px; /* Ensure it doesn't get too narrow */
            max-width: 900px; /* Limit max width to a nice value */
            width: 100%; /* Always responsive */
            box-sizing: border-box;
        }


        .day {
            background: #e0e5ec;
            border-radius: 12px;
            box-shadow: 5px 5px 10px #bec3c9, -5px -5px 10px #ffffff;
            padding: 15px;
            flex: 1 1 220px; /* Base size 220px, can shrink or grow */
            min-width: 200px; /* Never get smaller than this */
            max-width: 300px; /* Never get bigger than this */
            min-height: 180px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: space-around;
            text-align: center;
            box-sizing: border-box;
        }

        .day i {
            font-size: 36px;
        }
    </style>
</head>
<body>

<div class="card" id="currentWeather">
    <h2 id="location">Loading...</h2>
    <i id="currentIcon" class="wi" style="font-size: 3.0em"></i>
    <div id="currentTemp"></div>
    <div id="currentDesc"></div>
    <div id="currentDate"></div>
</div>

<div class="forecast" id="forecast">
    <!-- Forecast days will be added here -->
</div>
<button id="toggleUnits" style="margin: 10px; padding: 10px 20px; border-radius: 8px; border: none; background: #e0e5ec; box-shadow: 5px 5px 10px #bec3c9, -5px -5px 10px #ffffff; cursor: pointer;">
    Toggle °C / °F
</button>

<script>
function mapWeatherCodeToIcon(description) {
    if (description.includes("Clear")) return "wi-day-sunny";
    if (description.includes("Partly")) return "wi-day-cloudy";
    if (description.includes("Fog")) return "wi-fog";
    if (description.includes("Drizzle")) return "wi-sprinkle";
    if (description.includes("Rain")) return "wi-rain";
    if (description.includes("Snow")) return "wi-snow";
    if (description.includes("Thunderstorm")) return "wi-thunderstorm";
    return "wi-na";
}

function celsiusToFahrenheit(c) {
    return (c * 9/5) + 32;
}

let weatherData = null;
let useFahrenheit = true;
let latitude = null;
let longitude = null;

function fetchWeather() {
    fetch('/api/v1/weather')
        .then(response => response.json())
        .then(data => {
            console.log("Weather data:", data);

            weatherData = data;       // Store data
            renderWeather();          // Render using current unit (°C or °F)
        })
        .catch(error => {
            console.error('Error fetching weather:', error);
            document.getElementById('location').textContent = "Unavailable";
            document.getElementById('currentTemp').textContent = "--";
            document.getElementById('currentDesc').textContent = "--";
            document.getElementById('currentDate').textContent = "--";
            document.getElementById('currentIcon').className = 'wi';
        });
}

function renderWeather() {
    if (!weatherData) return;

    document.getElementById('location').innerText = weatherData.location;

    document.getElementById('currentIcon').className =
    weatherData.current && weatherData.current.icon
        ? `wi ${weatherData.current.icon}`
        : 'wi wi-na';

    let currentTemp = weatherData.current.temperature;
    let unit = "°C";
    if (useFahrenheit) {
        currentTemp = celsiusToFahrenheit(currentTemp);
        unit = "°F";
    }

    document.getElementById('currentTemp').innerText = `${Math.round(currentTemp)}${unit}`;
    document.getElementById('currentDesc').innerText = weatherData.current.description;
    document.getElementById('currentDate').innerText = new Date(weatherData.current.date).toLocaleString();

    const forecastDiv = document.getElementById('forecast');
    forecastDiv.innerHTML = '';

    if (weatherData.forecast) {
        weatherData.forecast.forEach(day => {
            let high = day.high;
            let low = day.low;
            if (useFahrenheit) {
                high = celsiusToFahrenheit(high);
                low = celsiusToFahrenheit(low);
            }

            const dayDiv = document.createElement('div');
            dayDiv.className = 'day';
            dayDiv.innerHTML = `
                <div>${new Date(day.date + "T00:00:00").toLocaleDateString()}</div>
                <i class="wi ${mapWeatherCodeToIcon(day.description)}"></i>
                <div>${Math.round(high)}${unit} / ${Math.round(low)}${unit}</div>
                <div>${day.description}</div>
            `;
            forecastDiv.appendChild(dayDiv);
        });
    }
}

function toggleUnits() {
    useFahrenheit = !useFahrenheit;
    renderWeather();
}

function detectLocationAndFetch() {
    fetchWeather();
}

document.addEventListener('DOMContentLoaded', () => {
    detectLocationAndFetch();
    const toggleButton = document.getElementById('toggleUnits');
    if (toggleButton) {
        toggleButton.addEventListener('click', toggleUnits);
    }
});

</script>

</body>
</html>
)rawliteral";