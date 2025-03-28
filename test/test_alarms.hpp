#ifndef TEST_ALARMS_H
#define TESTA_ALARMS_H

#include "ds3231/ds3231.h"
#include "task.hpp"

#include "FreeRTOS.h"
#include "Timers.h"


class TestAlarms : public Task {
    public:

    /***
	 * Constructor
	 */
	TestAlarms(const char * taskName, uint32_t stackSize = configMINIMAL_STACK_SIZE, uint8_t priority = TaskPrio_Low) : Task(taskName, stackSize, priority) {

	};

	/***
	 * Destructor
	 */
	virtual ~TestAlarms();

    static TestAlarms* getInstance( );

    void run(void* data); // main run loop

    static void setRTC(int status, time_t *result);
    static void (*_RTC_set_callback)(void);
    static void ds3231_interrupt_callback(uint, uint32_t);
    static void vTimerCallback30SecExpired(TimerHandle_t pxTimer);
    static ds3231_t _ds3231;

    static time_t get_seconds_from_datetime_t(datetime_t t);
    static bool set_system_time_from_RTC();
    static void print_c_time();

    private:
        static TestAlarms* m_instance;
        void validateDS3231Time();
        static void validateTime();
        static void setDS3231RTC(ds3231_data_t* t);
        static void setDS3231RTC(time_t* t);

        static void alarm_callback(void);
        static int set_rtc_alarm(datetime_t* t);

};

#endif