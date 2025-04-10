// TimeManager.cpp
#include "TimeManager.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/sntp.h"
#include <ctime>
#include <iostream>
#include "FreeRTOS.h"
#include "task.h"
#include "AppContext.h"
#include <hardware/rtc.h>

// Set system time from SNTP
extern "C" void sntp_set_system_time(uint32_t sec) {
    // Delegate to your own logic
    AppContext::getInstance().getService<TimeManager>()->setTimeFromEpoch(sec);
}
TimeManager& TimeManager::getInstance() {
    static TimeManager instance;
    return instance;
}

void TimeManager::initNtpClient() {
    if (!sntp_enabled()) {
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_init();
    }
}

void TimeManager::syncTimeWithNtp(int timeoutSeconds) {
    initNtpClient();
    std::cout << "Waiting for NTP time sync..." << std::endl;

    // Wait up to timeoutSeconds for time to be set
    time_t now = 0;
    int waited = 0;
    while (waited < timeoutSeconds) {
        time(&now);
        if (now > 1670000000) {  // sanity check: after 2022
            std::cout << "NTP time acquired: " << ctime(&now);
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        waited++;
    }

    std::cerr << "NTP time sync failed after " << timeoutSeconds << " seconds" << std::endl;
}

void TimeManager::setTimeFromEpoch(uint32_t epochSeconds) {
    time_t rawtime = epochSeconds;
    struct tm *ptm = gmtime(&rawtime);

    if (!ptm) return;

    datetime_t dt = {
        .year  = static_cast<int16_t>(ptm->tm_year + 1900),
        .month = static_cast<int8_t>(ptm->tm_mon + 1),
        .day   = static_cast<int8_t>(ptm->tm_mday),
        .dotw  = static_cast<int8_t>(ptm->tm_wday),
        .hour  = static_cast<int8_t>(ptm->tm_hour),
        .min   = static_cast<int8_t>(ptm->tm_min),
        .sec   = static_cast<int8_t>(ptm->tm_sec),
    };

    rtc_set_datetime(&dt);
}
