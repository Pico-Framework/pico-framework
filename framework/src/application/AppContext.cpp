#include "AppContext.h"
#include "FatFsStorageManager.h"
#include "TimeManager.h"
#include "JwtAuthenticator.h"

AppContext& AppContext::getInstance() {
    static AppContext instance;
    return instance;
}

void AppContext::initFrameworkServices() {
    static FatFsStorageManager fatFs;
    static TimeManager timeMgr;
    static JwtAuthenticator jwt;

    registerService(&fatFs);
    registerService(&timeMgr);
    registerService(&jwt);
}

// No need to define ServiceHolder instances explicitly unless you want
// to force-instantiation for linker visibility in rare cases.
