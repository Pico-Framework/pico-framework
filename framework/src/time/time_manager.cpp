/**
 * @file time_manager.cpp
 * @author Ian Archbell
 * @brief Provides time management services for the application
 * @version 0.1
 * @date 2025-01-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <FreeRTOS.h>
#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/rtc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#include "network.hpp"
#include "ntp_client.hpp"
#include "time_manager.hpp"
#include "logger.hpp"
#include "pico_time.hpp"
#include "controller.hpp"

#include "ds3231.hpp"

#if NDEBUG
#define TRACE_PRINTF(fmt, args...) // Disable tracing
#else
#define TRACE_PRINTF printf // Trace with printf
#endif

TimeManager* TimeManager::m_instance = NULL;
uint32_t TimeManager::_status = 0;

ds3231_t TimeManager::_ds3231 = {};

TimeManager* TimeManager::getInstance(){

    if(m_instance == NULL)
    {
        m_instance = new TimeManager("TimeManager");
    }
    return m_instance;
}

/***
 * Destructor
 */
TimeManager::~TimeManager() {
	stop();
}

/***
  * Main Run Task for agent - wait for something to do
  */
 void TimeManager::run(void* callback){
    printf("TimeManager running\n");
    bool status = false;
    // right now this stops everything waiting for wifi but it won't when we have framework finish
    while (!status){ 
        //status = Network::getLinkStatus();
        vTaskDelay(pdMS_TO_TICKS(3000));   
    } 
    // we'll be looking for notifications to do something
    while (1){
         vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TimeManager::validateDS3231Time(){

    ds3231_data_t read_data;
    /* Read the time registers of DS3231. */
    if(ds3231_read_current_time(&_ds3231, &read_data)) {
        printf("No data is received\n");
    } else {
        printf("DS3231 time: ");
        PicoTime::getInstance().print(&read_data);
        _status |= DS3231_TIMER_VALID;
    }
}

bool TimeManager:: setDS3231RTC(ds3231_data_t* ds3231_data){

    DS3231 ds3231;
    if (!ds3231.setTime(ds3231_data))
        return false;

    vTaskDelay(pdMS_TO_TICKS(1000));

    ds3231_data_t read_data;

    if(ds3231.getTime(&read_data)) {
        printf("No data received from ds3231\n");
        return false;
    } else {
        printf("DS3231: ");
        PicoTime::getInstance().print(&read_data);
        _status |= RTC_TIMER_VALID;
        return true;
    }
}

/**
 * 
 * Callback to set the RTC using the result from NPT
 * 
*/
void TimeManager::setRTC(int status, time_t *result) {

    printf("sizeof(time_t) = %zu bytes\n", sizeof(time_t));

    if (!result) {
        printf("setRTC received NULL result pointer\n");
        return;
    }

    printf("setRTC received time_t*: %p, value: %ld\n", result, *result);

    if (*result < 1609459200) {  // Ignore dates before 2021
        printf("Invalid NTP time received: %ld\n", *result);
        return;
    }

    printf("Before assignment, *result = %ld\n", *result);

    time_t safe_result = *result;  // Copy value (This is failing!)

    printf("safe_result after assignment: %ld\n", safe_result);

    if (safe_result == 0) {
        printf("ERROR: safe_result became 0 after assignment!\n");
    }

    // Before setting the RTC, set the system time
    if (PicoTime::getInstance().setSystemTime(safe_result)) {
        _status |= SYSTEM_TIMER_VALID;
    } else {
        printf("Failed to set system time\n");
    }

    printf("C time after system time set: ");
    PicoTime::getInstance().printSystemTime();
}
