/**
 * @file AppContext.cpp
 * @author Ian Archbell
 * @brief Implementation of the AppContext service locator.
 * 
 * Initializes and exposes shared service instances, including persistent file storage.
 * This file provides lazy initialization of FatFsStorageManager.
 * 
 * @version 0.1
 * @date 2025-03-31
 * 
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "framework_config.h"
#include "DebugTrace.h"
TRACE_INIT(AppContext);

#include "AppContext.h"
#include "FatFsStorageManager.h"
#include "JwtAuthenticator.h"

FatFsStorageManager* AppContext::fatFs = nullptr;

AppContext& AppContext::getInstance() {
    static AppContext instance;
    return instance;
}

FatFsStorageManager* AppContext::getFatFsStorage() {
    if (!fatFs) {
        static FatFsStorageManager instance;
        fatFs = &instance;
        TRACE("FatFsStorageManager initialized");
        fatFs->mount();  // Optional: auto-mount here
    }
    return fatFs;
}

#ifdef PICO_HTTP_ENABLE_JWT
JwtAuthenticator* AppContext::jwtAuth = nullptr;

JwtAuthenticator* AppContext::getJwtAuthenticator() {
    if (!jwtAuth) {
        jwtAuth = &JwtAuthenticator::getInstance(); 
        jwtAuth->init(JWT_SECRET, 3600);            
        TRACE("JwtAuthenticator initialized");
    }
    return jwtAuth;
}
#endif



    