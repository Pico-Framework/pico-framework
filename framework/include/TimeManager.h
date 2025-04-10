// TimeManager.h
#pragma once

class TimeManager {
public:
    static TimeManager& getInstance();

    void syncTimeWithNtp(int timeoutSeconds = 10);

private:
    TimeManager() = default;
    void initNtpClient();
};
