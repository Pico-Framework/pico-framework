/**
 * @file ds3231.cpp
 * @author Ian Archbell
 * @brief Wrapper class for the ds3231
 * @version 0.1
 * @date 2025-01-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "pico_time.hpp"
#include "ds3231.hpp"

bool DS3231::isEnabled(){

    uint8_t rxdata;
    absolute_time_t timeout = make_timeout_time_ms(100);	
    int ret = i2c_read_blocking_until(_ds3231.i2c, _ds3231.ds3231_addr, &rxdata, 1, false, timeout );
    return (ret < 0 ? false : true);

}

bool DS3231::i2c_init(){
    if (!isEnabled())
        return false;
    /* Initiliaze I2C line. */
    gpio_init(_sda_pin); // really should be in ds3231.c
    gpio_init(_scl_pin); // really should be in ds3231.c
    gpio_set_function(_sda_pin, GPIO_FUNC_I2C); // really should be in ds3231.c
    gpio_set_function(_scl_pin, GPIO_FUNC_I2C); // really should be in ds3231.c
    //gpio_pull_up(sda_pin); // we have hardware pullup
    //gpio_pull_up(scl_pin); // we have hardware pullup
    return ::i2c_init(_ds3231.i2c, 400 * 1000);
}

bool DS3231::setTime(ds3231_data_t* ds3231_data){

    if (!i2c_init())
        return false;

    /* Update the DS3231 time registers with the desired time and set alarm 1 to send interrupt signal. */
    ds3231_configure_time(&_ds3231, ds3231_data);

    ds3231_data_t read_data;
    /* Read the time registers of DS3231. */
    if(ds3231_read_current_time(&_ds3231, &read_data)) {
        printf("No data is received\n");
    } else {
        printf("DS3231: ");
        PicoTime::getInstance().print(&read_data);
        return true;
    } 
    return false;      

}

bool DS3231::setAlarm1(ds3231_data_t* ds3231_data, uint32_t seconds, enum ALARM_1_MASKS mask, gpio_irq_callback_t ds3231_interrupt_callback){
    
    ds3231_alarm_1_t alarm;

    alarm.am_pm = ds3231_data->am_pm;
    alarm.seconds = ds3231_data->seconds + seconds; // alarm in n seconds
    alarm.minutes = ds3231_data->minutes;
    alarm.hours = ds3231_data->hours;
    alarm.date = ds3231_data->date;
    alarm.day = ds3231_data->day;
    if (alarm.seconds > 59){
        alarm.seconds -= 60;
        alarm.minutes += 1;
    }
    if (alarm.minutes > 59){
        alarm.minutes -= 60;
        alarm.hours += 1;
    }
    if (alarm.hours > 23){
        alarm.hours -= 24;
        alarm.date += 1;
    }
    // this is rough checking! Date could still be wrong (could be at end of month)

    printf("Setting DS3231 alarm time: %02u:%02u:%02u    %i    %02u\n", 
        alarm.hours, alarm.minutes, alarm.seconds, alarm.day, alarm.date);

    return setAlarm1(&alarm, mask, ds3231_interrupt_callback);
}


bool DS3231::setAlarm1(ds3231_alarm_1_t* alarm, enum ALARM_1_MASKS mask, gpio_irq_callback_t ds3231_interrupt_callback){
    i2c_init();
    ds3231_clear_alarm(&_ds3231); // ensure alarm not set
    ds3231_set_interrupt_callback_function(_int_pin, ds3231_interrupt_callback);
    ds3231_enable_alarm_interrupt(&_ds3231, true); // this should be true by default
    return  ds3231_set_alarm_1(&_ds3231, alarm, ON_MATCHING_SECOND_AND_MINUTE);
}

bool DS3231::getTime(ds3231_data_t* read_data){
    return ds3231_read_current_time(&_ds3231, read_data);
}


