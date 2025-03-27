#include "AppContext.h"
#include "FatFsStorageManager.h"

FatFsStorageManager* AppContext::getFatFsStorage() {
    if (!fatFs) {
        static FatFsStorageManager instance;
        fatFs = &instance;
        fatFs->mount();  // Optional: auto-mount here
    }
    return fatFs;
}

    