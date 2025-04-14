#include "AppContext.h"
#include "FatFsStorageManager.h"
#include "TimeManager.h"
#include "JwtAuthenticator.h"
#include "LittleFsStorageManager.h"
#include "EventManager.h"
#include "framework_config.h"
#include "DebugTrace.h"
TRACE_INIT(AppContext);

AppContext& AppContext::getInstance() {
    static AppContext instance;
    return instance;
}

void AppContext::initFrameworkServices() {
    #if PICO_HTTP_ENABLE_LITTLEFS
        static LittleFsStorageManager littlefs;
        registerService<StorageManager>(&littlefs);
        TRACE("[AppContext] Registered LittleFsStorageManager.\n");
    #else
        static FatFsStorageManager fatfs;
        registerService<StorageManager>(&fatfs);
        TRACE("[AppContext] Registered FatFsStorageManager.\n");
    #endif
        // Time manager (always present)
        static TimeManager timeMgr;
        registerService<TimeManager>(&timeMgr);
        // Event manager (always present)
        static EventManager eventMgr;
        registerService<EventManager>(&eventMgr);
    
    #if PICO_HTTP_ENABLE_JWT
        static JwtAuthenticator jwt;
        registerService<JwtAuthenticator>(&jwt);
    #endif
    }
    

// No need to define ServiceHolder instances explicitly unless you want
// to force-instantiation for linker visibility in rare cases.
