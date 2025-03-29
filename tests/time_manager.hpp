#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "lwip/udp.h"
#include "ds3231/ds3231.h"
#include "task.hpp"

#include "FreeRTOS.h"
#include "Timers.h"
#include "ntp_client.hpp"

#define SYSTEM_TIMER_VALID 0x01
#define RTC_TIMER_VALID 0x01
#define DS3231_TIMER_VALID 0x01

class TimeManager : public Task {
    public:

    /***
	 * Constructor
	 */
	TimeManager(const char * taskName, uint32_t stackSize = configMINIMAL_STACK_SIZE, uint8_t priority = TaskPrio_Low) : Task(taskName, stackSize, priority) {

	};

	/***
	 * Destructor
	 */
	virtual ~TimeManager();

    static TimeManager* getInstance( );
    
    void run(); // main run loop
    void run(void* data); // main run loop

    static void setRTC(int status, time_t *result);
    static void (*_RTC_set_callback)(void);
    static ds3231_t _ds3231;

    static bool set_system_time_from_RTC();

    static uint32_t getStatus(){ return _status;};

    private:
        static TimeManager* m_instance;
        void validateDS3231Time();
        static void validateTime();
        static bool setDS3231RTC(ds3231_data_t* t);
        static uint32_t _status;
};

#endif