const char *weather_html = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Weather Forecast</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <!-- Weather Icons CDN -->
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/weather-icons/2.0.10/css/weather-icons.min.css">
    <style>
        body {
            font-family: Arial, sans-serif;
            background: #e0e5ec;
            margin: 0;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: flex-start;
            min-height: 100vh;
        }

        .weather-container {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 20px;
            width: 100%;
            max-width: 900px;
            margin: 0 auto;
            padding: 10px;
            box-sizing: border-box;
        }

        .current-weather {
            background: #e0e5ec;
            border-radius: 20px;
            box-shadow: 5px 5px 10px #bec3c9, -5px -5px 10px #ffffff;
            padding: 20px;
            width: 100%;
            box-sizing: border-box;
            text-align: center;
        }

        .current-weather h2 {
            margin: 0 0 10px 0;
            font-size: 24px;
        }

        .current-weather p {
            margin: 5px 0;
            font-size: 18px;
        }

        .forecast {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 20px;
            width: 100%;
            max-width: 900px;
            box-sizing: border-box;
        }

        .day {
            background: #e0e5ec;
            border-radius: 12px;
            box-shadow: 5px 5px 10px #bec3c9, -5px -5px 10px #ffffff;
            padding: 15px;
            flex: 1 1 220px;
            min-width: 200px;
            max-width: 300px;
            min-height: 180px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: space-around;
            text-align: center;
            box-sizing: border-box;
        }

        .unit-toggle {
            margin-top: 10px;
            font-size: 14px;
            color: #555;
        }

        .unit-toggle button {
            background: #e0e5ec;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            margin-left: 10px;
            box-shadow: 3px 3px 6px #bec3c9, -3px -3px 6px #ffffff;
            cursor: pointer;
            font-size: 14px;
        }

        .unit-toggle button:hover {
            box-shadow: inset 3px 3px 6px #bec3c9, inset -3px -3px 6px #ffffff;
        }
    </style>
</head>
<body>

<div class="weather-container">
    <div class="current-weather" id="current-weather">
        Loading...
    </div>
    <div class="unit-toggle">
        Temperature:
        <button id="toggle-units">°C / °F</button>
    </div>
    <div class="forecast" id="forecast"></div>
</div>

<script>
    let useFahrenheit = false;

    async function fetchWeather() {
        try {
            const response = await fetch('/api/v1/weather');
            if (!response.ok) {
                throw new Error('Failed to fetch weather');
            }
            const weather = await response.json();
            renderWeather(weather);
        } catch (err) {
            console.error(err);
            document.getElementById('current-weather').innerText = "Failed to load weather data.";
        }
    }

    function renderWeather(weather) {
        const current = weather.current;
        const forecast = weather.forecast || [];

        const temperature = convertTemperature(current.temperature);

        document.getElementById('current-weather').innerHTML = `
            <h2>${weather.location}</h2>
            <i class="wi ${current.icon}" style="font-size:48px;"></i>
            <p>${temperature}° ${useFahrenheit ? 'F' : 'C'}</p>
            <p>${current.description}</p>
            <p>${new Date(current.date).toLocaleDateString()}</p>
        `;

        const forecastDiv = document.getElementById('forecast');
        forecastDiv.innerHTML = '';

        forecast.forEach(day => {
            const high = convertTemperature(day.high);
            const low = convertTemperature(day.low);

            const dayDiv = document.createElement('div');
            dayDiv.className = 'day';
            dayDiv.innerHTML = `
                <div><strong>${new Date(day.date).toLocaleDateString()}</strong></div>
                <i class="wi ${day.icon}" style="font-size:32px;"></i>
                <div>High: ${high}°</div>
                <div>Low: ${low}°</div>
                <div>${day.description}</div>
            `;
            forecastDiv.appendChild(dayDiv);
        });
    }

    function convertTemperature(tempCelsius) {
        if (useFahrenheit) {
            return Math.round((tempCelsius * 9/5) + 32);
        }
        return Math.round(tempCelsius);
    }

    document.getElementById('toggle-units').addEventListener('click', () => {
        useFahrenheit = !useFahrenheit;
        fetchWeather();
    });

    fetchWeather();
</script>

</body>
</html>

)rawliteral";