#include "framework/AppContext.h"
#include "time/TimeManager.h"
#include "http/JwtAuthenticator.h"
#include "events/EventManager.h"
#include "events/GpioEventManager.h"
// Right now its mandatory to have one StorageManager interface 
#if PICO_HTTP_ENABLE_LITTLEFS
    #include "storage/LittleFsStorageManager.h"
    #include "storage/JsonService.h"
#else  
    #include "storage/JsonService.h" 
    #include "storage/FatFsStorageManager.h"
#endif
#include "framework_config.h"
#include "DebugTrace.h"
TRACE_INIT(AppContext);

SemaphoreHandle_t AppContext::mutex = xSemaphoreCreateMutex();

AppContext& AppContext::getInstance() {
    static AppContext instance;
    return instance;
}

void AppContext::initFrameworkServices() {
    #if PICO_HTTP_ENABLE_LITTLEFS
        static LittleFsStorageManager littlefs;
        registerService<StorageManager>(&littlefs);
        static JsonService jsonService(&littlefs);
        registerService<JsonService>(&jsonService);
        TRACE("[AppContext] Registered LittleFsStorageManager.\n");
    #else
        static FatFsStorageManager fatfs;
        registerService<StorageManager>(&fatfs);
        static JsonService jsonService(&fatfs);
        registerService<JsonService>(&jsonService);
        TRACE("[AppContext] Registered FatFsStorageManager.\n");
    #endif
        // Time manager (always present)
        static TimeManager timeMgr;
        registerService<TimeManager>(&timeMgr);
        // Event manager (always present)
        static EventManager eventMgr;
        registerService<EventManager>(&eventMgr);
    #if defined(ENABLE_GPIO_EVENTS)
        static GpioEventManager gpioEventManager = GpioEventManager::getInstance();
        AppContext::registerService<GpioEventManager>(&gpioEventManager);
    #endif
    
    #if PICO_HTTP_ENABLE_JWT
        static JwtAuthenticator jwt;
        registerService<JwtAuthenticator>(&jwt);
    #endif
    }
    

// No need to define ServiceHolder instances explicitly unless you want
// to force-instantiation for linker visibility in rare cases.
