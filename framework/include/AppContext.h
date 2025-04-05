/**
 * @file AppContext.h
 * @author Ian Archbell
 * @brief Global application context accessor for shared services.
 *
 * Provides static access to key singleton services such as storage and timer management,
 * acting as a central point for service lookup in the PicoFramework application layer.
 * This will get extended over time to include more services as needed.
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#pragma once

#include "FatFsStorageManager.h"
#include "TimerService.h"

#ifdef PICO_HTTP_ENABLE_JWT
#include "JwtAuthenticator.h"
#endif

class AppContext
{
public:
    static AppContext &getInstance();
    static FatFsStorageManager *getFatFsStorage();

#ifdef PICO_HTTP_ENABLE_JWT
    static JwtAuthenticator *getJwtAuthenticator();
#endif

private:
    static FatFsStorageManager *fatFs;

#ifdef PICO_HTTP_ENABLE_JWT
    static JwtAuthenticator *jwtAuth;
#endif
};

#endif // APP_CONTEXT_H