// TimeManager.h
#pragma once
#include <cstdint>
#include <string>
#include <ctime>
#include <time.h>

class TimeManager {
public:

    TimeManager() = default;
    bool syncTimeWithNtp(int timeoutSeconds = 10);

    /// @brief Set the system time from an epoch timestamp (e.g. from SNTP)
    void setTimeFromEpoch(uint32_t epochSeconds);

    void fetchAndApplyTimezoneFromOpenMeteo(float lat, float lon, const std::string& tzName);
    void detectAndApplyTimezone();
    void applyFixedTimezoneOffset(int offsetSeconds, const char* stdName = "UTC", const char* dstName = "UTC");
    int getTimezoneOffset() const { return timezoneOffsetSeconds; }
    const char* getTimezoneName() const { return timezoneName.c_str(); }
    std::string formatTimeWithZone(time_t rawTime) const;
    std::string currentTimeForTrace() const;
    void setTime(timespec* ts);

private:

    void initNtpClient();
    bool getLocationFromIp(std::string& tzName, float& lat, float& lon);
    int timezoneOffsetSeconds = 0;
    std::string timezoneName = "UTC";
};
