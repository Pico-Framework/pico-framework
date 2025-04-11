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

// Set system time from SNTP - this callback is defined in lwipopts.h
extern "C" void sntp_set_system_time(uint32_t sec) {

    printf("[SNTP] Setting system time to: %u\n", sec);
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

static time_t get_seconds_from_datetime_t() {
    
    struct tm timeinfo;
    aon_timer_get_time_calendar(&timeinfo);

    return mktime(&timeinfo);             // Converts to time_t and updates libc time state
}

void TimeManager::setTimeFromEpoch(uint32_t epoch) {
    
    struct timespec ts = {
        .tv_sec = epoch,
        .tv_nsec = 0
    }; 

    struct timeval tv = {
        .tv_sec = get_seconds_from_datetime_t(),
        .tv_usec = 0
    }; 
    
    if (aon_timer_start(&ts)) {
        printf("[TimeManager] AON timer started at epoch %u.\n", epoch);
        PicoTime::printNow(); 
        
        settimeofday(&tv, nullptr); 
        printf("[TimeManager] AON timer started at epoch %u.\n", epoch);  
    } else {
        printf("[TimeManager] Failed to start AON timer.\n");
    }
}
    