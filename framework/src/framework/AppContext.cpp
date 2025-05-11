#include "framework/AppContext.h"
#include "time/TimeManager.h"
#include "http/JwtAuthenticator.h"
#include "events/EventManager.h"
#include "events/TimerService.h"
#include "events/GpioEventManager.h"
#include "utility/logger.h"
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
    printf("[AppContext] Initializing framework services...\n");
    #if PICO_HTTP_ENABLE_LITTLEFS
        TRACE("[AppContext] Initializing LittleFS storage manager.\n");
        static LittleFsStorageManager littlefs;
        registerService<StorageManager>(&littlefs);
        TRACE("[AppContext] LittleFS storage manager registered.\n");
        static JsonService jsonService(&littlefs);
        registerService<JsonService>(&jsonService);
        TRACE("[AppContext] Registered JsonService.\n");
        
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
        TRACE("[AppContext] Registered TimeManager.\n");
        // Event manager (always present)
        static EventManager eventMgr;
        registerService<EventManager>(&eventMgr);
        TRACE("[AppContext] Registered EventManager.\n");
        // Timer service (always present)
        static TimerService timerService;
        registerService<TimerService>(&timerService);
        TRACE("[AppContext] Registered TimerService.\n");
        static Logger logger;
        registerService<Logger>(&logger);
        TRACE("[AppContext] Registered Logger.\n");
    #if defined(ENABLE_GPIO_EVENTS)
        static GpioEventManager gpioEventManager = GpioEventManager::getInstance();
        AppContext::registerService<GpioEventManager>(&gpioEventManager);
        TRACE("[AppContext] Registered GpioEventManager.\n");
    #endif
    
    #if PICO_HTTP_ENABLE_JWT
        static JwtAuthenticator jwt;
        registerService<JwtAuthenticator>(&jwt);
        TRACE("[AppContext] Registered JwtAuthenticator.\n");
    #endif
    }

// No need to define ServiceHolder instances explicitly unless you want
// to force-instantiation for linker visibility in rare cases.
