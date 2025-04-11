// TimeManager.h
#include <cstdint>
#pragma once

class TimeManager {
public:
    static TimeManager& getInstance();

    void syncTimeWithNtp(int timeoutSeconds = 10);

    /// @brief Set the system time from an epoch timestamp (e.g. from SNTP)
    void setTimeFromEpoch(uint32_t epochSeconds);

private:
    TimeManager() = default;
    void initNtpClient();
    void fetchAndApplyTimezoneFromWorldTimeApi();
};
