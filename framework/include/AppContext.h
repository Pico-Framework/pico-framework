#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H   

#pragma once
#include "FatFsStorageManager.h"

class AppContext {
public:
    static FatFsStorageManager* getFatFsStorage();

private:
    static inline FatFsStorageManager* fatFs = nullptr;
};

#endif
