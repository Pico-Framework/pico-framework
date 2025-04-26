#pragma once

#include <string>
#include <vector>

/**
 * @brief Lightweight structure for current weather snapshot.
 */
struct WeatherSnapshot {
    std::string time;
    float temperature = 0.0f;
};

/**
 * @brief Lightweight structure for a single day's forecast.
 */
struct DailyForecast {
    std::string date;
    float maxTemp = 0.0f;
    float minTemp = 0.0f;
    int dominantWeatherCode = 0;
};

/**
 * @brief Weather data parsing and helpers.
 */
class Weather
{
public:
    WeatherSnapshot parseWeatherSnapshot(const std::string& body);
    std::vector<DailyForecast> parseDailySummary(const std::string& body, int days);

    static std::string describeWeatherCode(int code);
};
