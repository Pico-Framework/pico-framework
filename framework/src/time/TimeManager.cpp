// TimeManager.cpp
#include "time/TimeManager.h"

#include <ctime>
#include <iostream>
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include "pico/aon_timer.h"
#include <lwip/apps/sntp.h>
#include <FreeRTOS.h>
#include <task.h>
#include "events/EventManager.h"
#include "events/Notification.h"
#include "time/PicoTime.h"
#include "framework/AppContext.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "framework_config.h"
#include "DebugTrace.h"


#if defined(PICO_RP2040)
#include "hardware/rtc.h"
#endif
#include "time/PicoTime.h"


#ifndef PICO_HTTP_ENABLE_LITTLEFS
#include "FreeRTOS_time.h"
#endif

TRACE_INIT(TimeManager);

// Set system time from SNTP - this callback is defined in lwipopts.h
extern "C" void sntp_set_system_time(uint32_t sec)
{
    TRACE("[SNTP] callback made\n");
    AppContext::get<TimeManager>()->setTimeFromEpoch(sec);
}

void TimeManager::initNtpClient()
{
    if (!sntp_enabled())
    {
        TRACE("[Time Manager] Initializing SNTP client...\n");
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_init();
    }
}

bool TimeManager::syncTimeWithNtp(int timeoutSeconds)
{   
    printf("[Time Manager] Waiting for NTP time sync...\n");

    // Wait up to timeoutSeconds for time to be set
    int waited = 0;
    while (waited < timeoutSeconds)
    {
        if(timeSynced)
        {
            timespec ts;
            if(aon_timer_get_time(&ts)){
                if (ts.tv_sec > 1735689600)
                { // sanity check: after Jan 1 2025
                    printf("[Time Manager] NTP time acquired successfuly\n");
                    return true;
                }
                else
                {
                    printf("[Time Manager] System time epoch is invalid: %ld\n", ts.tv_sec);
                    return false;
                }
            }
            else
            {
                printf("[Time Manager] Failed to get system time from AON timer.\n");
                return false;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        waited++;
    }

    // Timeout occurred
    if (!isTimeValid()) {
        printf("[Time Manager] NTP sync failed and no valid time source available.\n");
        Event event;
        event.notification = SystemNotification::TimeInvalid;
        AppContext::get<EventManager>()->postEvent(event);
    }
    else {
        printf("[Time Manager] NTP sync failed, but AON timer is running — time still valid.\n");
    }
    return false;
}

void TimeManager::setTimeFromEpoch(uint32_t epoch)
{
    TRACE("[TimeManager] Setting system time from epoch: %u\n", epoch);
    struct timespec ts = {
        .tv_sec = epoch,
        .tv_nsec = 0};
    setTime(&ts);
    timeSynced = true;
    Event event;
    event.notification = SystemNotification::TimeSync;
    AppContext::get<EventManager>()->postEvent(event);
    TRACE("[TimeManager] System time set to: %s\n", ctime(&ts.tv_sec));
}

/**
 * @brief Set the system time using a timespec structure.
 *
 * This function sets the system time using the provided timespec structure.
 * It initializes the RTC if available and sets the always-on timer to the specified time.
 * If the always-on timer is already running, it does nothing.
 *
 * @param ts Pointer to a timespec structure containing the time to set.
 */
void TimeManager::setTime(timespec *ts)
{
    if (ts == nullptr)
    {
        printf("[TimeManager] Invalid timespec provided.\n");
        return;
    }

    // Initialize the RTC if necessary

    // Set the system time using the provided timespec
    TRACE("[TimeManager] Setting time: %ld seconds, %ld nanoseconds\n", ts->tv_sec, ts->tv_nsec);
        // if the AON timer is not running, start it
    if (!aon_timer_is_running()){
        printf("[TimeManager] AON timer is not running, starting it...\n");
        aon_timer_start(ts);
    }
    else{
        // if the AON timer is running, set the time
        printf("[TimeManager] AON timer is running, syncing time...\n");
        aon_timer_set_time(ts);
    }
    if(!aon_timer_get_time(ts))
    {
        printf("[TimeManager] Failed to get system time from AON timer.\n");
        return;
    }
    else{
        TRACE("[TimeManager] System time set to: %ld seconds, %ld nanoseconds\n", ts->tv_sec, ts->tv_nsec);
        time_t secs = PicoTime::now();
        TRACE("[TimeManager] Current time: %s\n", ctime(&secs));
        AppContext::get<EventManager>()->postEvent({SystemNotification::TimeValid});
    }
}

void TimeManager::applyFixedTimezoneOffset(int offsetSeconds, const char *stdName, const char *dstName)
{
    TRACE("[TimeManager] Setting timezone offset: %d seconds\n", offsetSeconds);
    printf("[TimeManager] Standard timezone: %s, DST timezone: %s\n", stdName, dstName);
    timezoneOffsetSeconds = offsetSeconds;
    timezoneName = stdName;

    // Log for trace/debug purposes
    printf("[TimeManager] Timezone set to UTC %+d:00 (%s)\n",
           offsetSeconds / 3600, stdName);
}

bool TimeManager::getLocationFromIp(std::string &tzName, float &lat, float &lon)
{
    HttpRequest req;
    HttpResponse res = req.get("http://ip-api.com/json");

    const std::string &body = res.getBody();
    if (body.empty())
    {
        printf("[TimeManager] Failed to get IP geolocation.\n");
        return false;
    }

    // Parse timezone
    size_t tzPos = body.find("\"timezone\":\"");
    if (tzPos != std::string::npos)
    {
        tzPos += strlen("\"timezone\":\"");
        size_t end = body.find('"', tzPos);
        if (end != std::string::npos)
        {
            tzName = body.substr(tzPos, end - tzPos);
        }
    }
    else
    {
        tzName = "UTC";
    }

    // Parse lat/lon
    size_t latPos = body.find("\"lat\":");
    size_t lonPos = body.find("\"lon\":");
    if (latPos == std::string::npos || lonPos == std::string::npos)
    {
        printf("[TimeManager] lat/lon not found. Using defaults.\n");
        lat = 0.0f;
        lon = 0.0f;
        return false;
    }

    lat = std::stof(body.substr(latPos + 6));
    lon = std::stof(body.substr(lonPos + 6));
    return true;
}

void TimeManager::fetchAndApplyTimezoneFromOpenMeteo(float lat, float lon, const std::string &tzName)
{
    char url[256];
    snprintf(url, sizeof(url),
             "http://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current_weather=true&timezone=auto",
             lat, lon);

    TRACE("url: %s\n", url);

    HttpRequest req;
    HttpResponse res = req.get(url);

    const std::string &body = res.getBody();
    TRACE("[TimeManager] Open-Meteo response: %s\n", body.c_str());
    TRACE("Status code: %d\n", res.getStatusCode());
    if (body.empty())
    {
        printf("[TimeManager] Open-Meteo response is empty.\n");
        applyFixedTimezoneOffset(0, tzName.c_str(), tzName.c_str());
        return;
    }

    size_t offsetPos = body.find("\"utc_offset_seconds\":");
    int offsetSeconds = 0;
    if (offsetPos != std::string::npos)
    {
        offsetPos += strlen("\"utc_offset_seconds\":");
        size_t end = body.find(',', offsetPos);
        if (end == std::string::npos)
            end = body.size();
        offsetSeconds = std::stoi(body.substr(offsetPos, end - offsetPos));
    }
    TRACE("[TimeManager] Timezone: %s, UTC offset: %d sec\n", tzName.c_str(), offsetSeconds);
    applyFixedTimezoneOffset(offsetSeconds, tzName.c_str(), tzName.c_str());
}

void TimeManager::detectAndApplyTimezone()
{
    std::string tzName = "UTC";
    float lat = 0.0f, lon = 0.0f;

    if (getLocationFromIp(tzName, lat, lon))
    {
        fetchAndApplyTimezoneFromOpenMeteo(lat, lon, tzName);
    }
    else
    {
        printf("[TimeManager] Could not determine location. Using default UTC.\n");
        applyFixedTimezoneOffset(0, tzName.c_str(), tzName.c_str());
    }
}

std::string TimeManager::formatTimeWithZone(time_t utcTime) const
{
    time_t localTime = utcTime + timezoneOffsetSeconds;
    TRACE("[TimeManager : formatWithZone] Offset: %d\n", timezoneOffsetSeconds);
    TRACE("Timzeone: %s\n", timezoneName.c_str());
    struct tm tmBuf;
    gmtime_r(&localTime, &tmBuf); // Adjusted time treated as UTC

    char timeBuf[16] = {0};
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tmBuf);
    TRACE("[TimeManager : formatWithZone] Time: %s\n", timeBuf);

    const char *zone = timezoneName.empty() ? "?" : timezoneName.c_str();

    char formatted[48] = {0};
    snprintf(formatted, sizeof(formatted), "[%s %s]", timeBuf, zone);

    return std::string(formatted);
}

std::string TimeManager::currentTimeForTrace() const
{
    return formatTimeWithZone(time(nullptr));
}

void TimeManager::start() {
    if (isTimeValid()) {
        AppContext::get<EventManager>()->postEvent({SystemNotification::TimeValid});
    }
}

void TimeManager::onNetworkReady()
{
    initNtpClient(); // Initialize SNTP client - this will automatically cause a timesync event when that happens
    if (!isTimeValid())
    {
        syncTimeWithNtp(NTP_TIMEOUT_SECONDS);
    }

}

void TimeManager::onHttpServerStarted(){
    detectAndApplyTimezone(); // Ensure timezone is set when HTTP server starts
}
