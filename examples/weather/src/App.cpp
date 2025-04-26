#include "App.h"
#include <iostream>
#include "events/Notification.h"
#include "framework/AppContext.h"
#include "http/HttpServer.h"
#include "weather_html.h"
#include "weather.h"


App::App(int port) : FrameworkApp(port, "AppTask", 1024, 1) {
    
}

void App::initRoutes()
{
    FrameworkApp::initRoutes();

    router.addRoute("GET", "/", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        res.send(weather_html);
    });

    router.addRoute("GET", "/api/v1/weather", [this](HttpRequest &req, HttpResponse &res, const RouteMatch &match)
    {
        this->handleWeatherRequest(req, res);
    });
}

void App::onStart() {
    
    FrameworkApp::onStart();

    printf("[MyApp] Starting Storage App...\n");

    // Wait for network
    printf("[MyApp] Waiting for network...\n");
    waitFor(SystemNotification::NetworkReady);

    // Start server once network is ready
    printf("[MyApp] Network ready. Starting HTTP server...\n");
    server.start();

}

void App::handleWeatherRequest(HttpRequest &req, HttpResponse &res)
{
    nlohmann::json weatherJson;

    double lat = latitude;
    double lon = longitude;


    // Check if browser provided lat/lon via query
    auto params = req.getQueryParams();
    auto latIt = params.find("lat");
    auto lonIt = params.find("lon");

    if (latIt != params.end() && lonIt != params.end())
    {
        lat = std::stod(latIt->second);
        lon = std::stod(lonIt->second);
    }
    else
    {
        // Try to get location from IP geolocation
        std::string tzName;
        if (!getLocationFromIp(tzName, lat, lon))
        {
            printf("[WeatherApp] Failed to get location from IP. Using defaults.\n");
        }
        else
        {
            printf("[WeatherApp] Using IP geolocation: %s (lat: %.4f, lon: %.4f)\n", tzName.c_str(), lat, lon);
        }
    }

    if (fetchWeatherSnapshot(weatherJson, lat, lon))
    {
        res.json(weatherJson);
    }
    else
    {
        res.sendError(500, "Failed to fetch weather data");
    }
}

bool App::getLocationFromIp(std::string &tzName, double &lat, double &lon)
{
    HttpRequest req;
    HttpResponse res = req.get("http://ip-api.com/json");

    const std::string &body = res.getBody();
    if (body.empty())
    {
        printf("[WeatherApp] Failed to get IP geolocation (empty body).\n");
        return false;
    }

    auto cityPos = body.find("\"city\":\"");
    auto regionPos = body.find("\"regionName\":\"");
    if (cityPos != std::string::npos && regionPos != std::string::npos)
    {
        cityPos += strlen("\"city\":\"");
        regionPos += strlen("\"regionName\":\"");
        auto cityEnd = body.find('"', cityPos);
        auto regionEnd = body.find('"', regionPos);
        if (cityEnd != std::string::npos && regionEnd != std::string::npos)
        {
            location = body.substr(cityPos, cityEnd - cityPos) + ", " +
                    body.substr(regionPos, regionEnd - regionPos);
        }
    }

    // Parse timezone
    auto tzPos = body.find("\"timezone\":\"");
    if (tzPos != std::string::npos)
    {
        tzPos += strlen("\"timezone\":\"");
        auto end = body.find('"', tzPos);
        if (end != std::string::npos)
        {
            tzName = body.substr(tzPos, end - tzPos);
        }
        else
        {
            tzName = "UTC";
        }
    }
    else
    {
        tzName = "UTC";
    }

    // Parse latitude
    auto latPos = body.find("\"lat\":");
    if (latPos != std::string::npos)
    {
        lat = std::stof(body.substr(latPos + 6));
    }
    else
    {
        lat = 0.0f;
    }

    // Parse longitude
    auto lonPos = body.find("\"lon\":");
    if (lonPos != std::string::npos)
    {
        lon = std::stof(body.substr(lonPos + 6));
    }
    else
    {
        lon = 0.0f;
    }

    if (lat == 0.0f && lon == 0.0f)
    {
        printf("[WeatherApp] lat/lon not found. Using defaults.\n");
        return false;
    }

    return true;
}


bool App::fetchWeatherSnapshot(nlohmann::json &weatherOut, double lat, double lon)
{
    std::string url = "http://api.open-meteo.com/v1/forecast?"
                      "latitude=" + std::to_string(lat) +
                      "&longitude=" + std::to_string(lon) +
                      "&current_weather=true"
                      "&daily=temperature_2m_max,temperature_2m_min,weathercode"
                      "&timezone=auto";

    HttpRequest request;
    request.setUri(url);
    request.setMethod("GET");
    request.setAcceptEncoding("identity"); // No compression

    HttpResponse response = request.send();
    if (!response.ok())
    {
        printf("[App] HTTP request failed: %s\n", response.getBody().c_str());
        return false;
    }

    Weather weather;
    WeatherSnapshot snapshot = weather.parseWeatherSnapshot(response.getBody());
    std::vector<DailyForecast> forecast = weather.parseDailySummary(response.getBody(), 3); // 3 days only

    // Build the lightweight JSON for frontend
    weatherOut["location"] = location; // Could use reverse geolocation later
    weatherOut["current"] = {
        {"temperature", snapshot.temperature},
        {"description", "Current conditions"}, // Could improve if we parse weathercode
        {"icon", "wi-day-sunny"},               // Dummy for now
        {"date", snapshot.time}
    };

    for (const auto &day : forecast)
    {
        weatherOut["forecast"].push_back({
            {"date", day.date},
            {"high", day.maxTemp},
            {"low", day.minTemp},
            {"description", Weather::describeWeatherCode(day.dominantWeatherCode)},
            {"icon", "wi-day-cloudy"} // Dummy until you map codes
        });
    }

    return true;
}
