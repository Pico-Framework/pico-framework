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
#include "pico/aon_timer.h"
#include "PicoTime.h"
#include "FreeRTOS.h"
#include "FreeRTOS_time.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

// Set system time from SNTP - this callback is defined in lwipopts.h
extern "C" void sntp_set_system_time(uint32_t sec) {

    printf("[SNTP] Setting system time to: %u\n", sec);
    AppContext::getInstance().getService<TimeManager>()->setTimeFromEpoch(sec);
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
    printf("Waiting for NTP time sync...\n");

    // Wait up to timeoutSeconds for time to be set
    time_t now = 0;
    int waited = 0;
    while (waited < timeoutSeconds) {
        time(&now);
        if (now > 1670000000) {  // sanity check: after 2022
            printf("NTP time acquired: ");
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        waited++;
    }

    std::cerr << "NTP time sync failed after " << timeoutSeconds << " seconds" << std::endl;
}

void TimeManager::setTimeFromEpoch(uint32_t epoch) {
    struct timespec ts = {
        .tv_sec = epoch,
        .tv_nsec = 0
    };
    struct timeval tv = {
        .tv_sec = epoch,
        .tv_usec = 0
    };

    FreeRTOS_time_init(); 
    settimeofday(&tv, NULL);  // this updates c system time
    setrtc(&ts);  //  This updates aon clock, FreeRTOS epochtime and starts the 1s update timer
    printf("[TimeManager] AON and FreeRTOS+FAT time system set via setrtc().\n");
}

void TimeManager::applyFixedTimezoneOffset(int offsetSeconds, const char* stdName, const char* dstName) {
    printf("[TimeManager] Setting timezone offset: %d seconds\n", offsetSeconds);
    printf("[TimeManager] Standard timezone: %s, DST timezone: %s\n", stdName, dstName);
    timezoneOffsetSeconds = offsetSeconds;
    timezoneName = stdName;

    // Log for trace/debug purposes
    printf("[TimeManager] Timezone set to UTC %+d:00 (%s)\n",
           offsetSeconds / 3600, stdName);
}

bool TimeManager::getLocationFromIp(std::string& tzName, float& lat, float& lon) {
    HttpRequest req;
    HttpResponse res = req.get("http://ip-api.com/json");

    const std::string& body = res.getBody();
    if (body.empty()) {
        printf("[TimeManager] Failed to get IP geolocation.\n");
        return false;
    }

    // Parse timezone
    size_t tzPos = body.find("\"timezone\":\"");
    if (tzPos != std::string::npos) {
        tzPos += strlen("\"timezone\":\"");
        size_t end = body.find('"', tzPos);
        if (end != std::string::npos) {
            tzName = body.substr(tzPos, end - tzPos);
        }
    } else {
        tzName = "UTC";
    }

    // Parse lat/lon
    size_t latPos = body.find("\"lat\":");
    size_t lonPos = body.find("\"lon\":");
    if (latPos == std::string::npos || lonPos == std::string::npos) {
        printf("[TimeManager] lat/lon not found. Using defaults.\n");
        lat = 0.0f;
        lon = 0.0f;
        return false;
    }

    lat = std::stof(body.substr(latPos + 6));
    lon = std::stof(body.substr(lonPos + 6));
    return true;
}

void TimeManager::fetchAndApplyTimezoneFromOpenMeteo(float lat, float lon, const std::string& tzName) {
    char url[256];
    snprintf(url, sizeof(url),
        "http://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current_weather=true&timezone=auto",
        lat, lon);

    printf("url: %s\n", url);    

    HttpRequest req;
    HttpResponse res = req.get(url);

    const std::string& body = res.getBody();
    printf("[TimeManager] Open-Meteo response: %s\n", body.c_str());
    printf("Status code: %d\n", res.getStatusCode());
    if (body.empty()) {
        printf("[TimeManager] Open-Meteo response is empty.\n");
        applyFixedTimezoneOffset(0, tzName.c_str(), tzName.c_str());
        return;
    }

    size_t offsetPos = body.find("\"utc_offset_seconds\":");
    int offsetSeconds = 0;
    if (offsetPos != std::string::npos) {
        offsetPos += strlen("\"utc_offset_seconds\":");
        size_t end = body.find(',', offsetPos);
        if (end == std::string::npos) end = body.size();
        offsetSeconds = std::stoi(body.substr(offsetPos, end - offsetPos));
    }
    printf("[TimeManager] Timezone: %s, UTC offset: %d sec\n", tzName.c_str(), offsetSeconds);
    applyFixedTimezoneOffset(offsetSeconds, tzName.c_str(), tzName.c_str());
}

void TimeManager::detectAndApplyTimezone() {
    std::string tzName = "UTC";
    float lat = 0.0f, lon = 0.0f;

    if (getLocationFromIp(tzName, lat, lon)) {
        fetchAndApplyTimezoneFromOpenMeteo(lat, lon, tzName);
    } else {
        printf("[TimeManager] Could not determine location. Using default UTC.\n");
        applyFixedTimezoneOffset(0, tzName.c_str(), tzName.c_str());
    }
}

std::string TimeManager::formatTimeWithZone(time_t utcTime) const {
    time_t localTime = utcTime + timezoneOffsetSeconds;
    printf("[TimeManager : formatWithZone] Offset: %d\n", timezoneOffsetSeconds);
    printf("Timzeone: %s\n", timezoneName.c_str());
    struct tm tmBuf;
    gmtime_r(&localTime, &tmBuf);  // Adjusted time treated as UTC

    char timeBuf[16] = {0};
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tmBuf);
    printf("[TimeManager : formatWithZone] Time: %s\n", timeBuf);

    const char* zone = timezoneName.empty() ? "?" : timezoneName.c_str();

    char formatted[48] = {0};
    snprintf(formatted, sizeof(formatted), "[%s %s]", timeBuf, zone);

    return std::string(formatted);
}


std::string TimeManager::currentTimeForTrace() const {
    return formatTimeWithZone(time(nullptr));
}

    