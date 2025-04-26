#include "weather.h"
#include "nlohmann/json.hpp"
#include <cstdio>

WeatherSnapshot Weather::parseWeatherSnapshot(const std::string& body)
{
    WeatherSnapshot snapshot;

    auto root = nlohmann::json::parse(body, nullptr, false);
    if (root.is_discarded()) {
        printf("[Weather] JSON parse error (snapshot)\n");
        return snapshot;
    }

    if (root.contains("current_weather"))
    {
        auto current = root["current_weather"];
        snapshot.temperature = current.value("temperature", 0.0f);
        snapshot.time = current.value("time", "");
    }
    else
    {
        printf("[Weather] Missing current_weather field\n");
    }

    return snapshot;
}

std::vector<DailyForecast> Weather::parseDailySummary(const std::string& body, int days)
{
    std::vector<DailyForecast> forecast;

    auto root = nlohmann::json::parse(body, nullptr, false);
    if (root.is_discarded()) {
        printf("[Weather] JSON parse error (daily)\n");
        return forecast;
    }

    if (root.contains("daily"))
    {
        auto daily = root["daily"];
        auto times = daily["time"];
        auto maxTemps = daily["temperature_2m_max"];
        auto minTemps = daily["temperature_2m_min"];
        auto codes = daily["weathercode"];

        int availableDays = std::min<int>({ int(times.size()), int(maxTemps.size()), int(minTemps.size()), int(codes.size()), days });

        for (int i = 0; i < availableDays; ++i)
        {
            DailyForecast df;
            df.date = times[i];
            df.maxTemp = maxTemps[i];
            df.minTemp = minTemps[i];
            df.dominantWeatherCode = codes[i];
            forecast.push_back(df);
        }
    }
    else
    {
        printf("[Weather] Missing daily field\n");
    }

    return forecast;
}

std::string Weather::describeWeatherCode(int code)
{
    if (code == 0) return "Clear Sky";
    if (code >= 1 && code <= 3) return "Partly Cloudy";
    if (code >= 45 && code <= 48) return "Fog";
    if (code >= 51 && code <= 57) return "Drizzle";
    if (code >= 61 && code <= 67) return "Rain";
    if (code >= 71 && code <= 77) return "Snow";
    if (code >= 80 && code <= 82) return "Rain Showers";
    if (code >= 85 && code <= 86) return "Snow Showers";
    if (code >= 95 && code <= 99) return "Thunderstorm";

    return "Unknown";
}