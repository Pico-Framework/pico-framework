// TimeManager.h
#pragma once
#include <cstdint>
#include <string>
#include <ctime>
#include <time.h>
#include "pico/aon_timer.h"
#include "events/Event.h"

class TimeManager {
public:

    TimeManager() = default;

    /**
     * @brief Initialize the time manager.
     * This sets up the SNTP client and attempts to sync time with NTP servers.
     * @return true if initialization was successful, false otherwise.
     * 
     */
    bool syncTimeWithNtp(int timeoutSeconds = 20); // sync retry window is 15 seconds

    /// @brief Set the system time from an epoch timestamp (e.g. from SNTP)
    void setTimeFromEpoch(uint32_t epochSeconds);

    /**
     * @brief Fetch and apply the timezone from OpenMeteo API using latitude and longitude.
     * @param lat Latitude of the location.
     * @param lon Longitude of the location.
     * @param tzName Optional timezone name to apply if available.
     */
    void fetchAndApplyTimezoneFromOpenMeteo(float lat, float lon, const std::string& tzName);

    /**
     * @brief Detect and apply the timezone based on the current location or IP address.
     * This will attempt to determine the timezone automatically.
     */
    void detectAndApplyTimezone();

    /**
     * @brief Apply a fixed timezone offset.
     * @param offsetSeconds Offset in seconds from UTC.
     * @param stdName Standard timezone name (default "UTC").
     * @param dstName Daylight Saving Time name (default "UTC").
     */
    void applyFixedTimezoneOffset(int offsetSeconds, const char* stdName = "UTC", const char* dstName = "UTC");

    /**
     * @brief Get the current timezone offset in seconds.
     * @return The timezone offset in seconds from UTC.
     */
    int getTimezoneOffset() const { return timezoneOffsetSeconds; }

    /**
     * @brief Get the name of the current timezone.
     * @return The name of the timezone as a string.
     */
    const char* getTimezoneName() const { return timezoneName.c_str(); }

    std::string formatTimeWithZone(time_t rawTime = 0) const;

    /**
     * @brief Get the current time formatted as a string.
     * This will return the current time timezone information.
     * @return A string representing the current time.
     */
    std::string currentTimeForTrace() const;

    /**
     * @brief Set the system time using a timespec structure.
     * This is useful for setting the time directly from a timespec object.
     * @param ts Pointer to a timespec structure containing the time to set.
     */
    void setTime(timespec* ts);

    /**
     * @brief Returns true if system time has been synced with NTP servers.
     * Note that sntp syncing is setup for every hour by default - this can be changed in framework_config.h
     * This retrieves the current time in a timespec format.
     * @return A timespec structure containing the current time.
     */
    bool isTimeSynced() const { return timeSynced; }

    bool isTimeValid() {
        return aon_timer_is_running(); // Direct real system call, no private bool
    }

    /**
     * @brief Start the time manager. It will check if time is valid and post event if it is
     */
    void start();

    /**
     * @brief hanles network ready event.
     * This is called when the network is ready and the time manager can start syncing time.
     */
    void onNetworkReady();

    void onHttpServerStarted();

private:
    bool timeSynced = false;
    void initNtpClient();
    bool getLocationFromIp(std::string& tzName, float& lat, float& lon);
    int timezoneOffsetSeconds = 0;
    std::string timezoneName = "UTC";
};
