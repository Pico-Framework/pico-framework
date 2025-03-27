#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H   

#pragma once

#include "FatFsStorageManager.h"
#include "TimerService.h"  


class AppContext {
public:
    static FatFsStorageManager* getFatFsStorage();
    TimerService& getTimerService(){
        return TimerService::instance();
    }

private:
    static inline FatFsStorageManager* fatFs = nullptr;
};

#endif


