#include "AppContext.h"
#include "FatFsStorageManager.h"
#include "TimeManager.h"
#include "JwtAuthenticator.h"

AppContext* AppContext::instance = nullptr;

AppContext& AppContext::getInstance() {
    if (!instance) {
        static AppContext ctx;
        instance = &ctx;
    }
    return *instance;
}

void AppContext::initFrameworkServices() {
    static FatFsStorageManager fatFs;
    static TimeManager timeMgr = TimeManager::getInstance();
    static JwtAuthenticator jwt = JwtAuthenticator::getInstance();

    REGISTER_SERVICE(FatFsStorageManager, &fatFs);
    registerService(&timeMgr);
    registerService(&jwt);
}

// No need to define ServiceHolder instances explicitly unless you want
// to force-instantiation for linker visibility in rare cases.
