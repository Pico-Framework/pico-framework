/**
 * @file FrameworkManager.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef FRAMEWORK_MANAGER_H
#define FRAMEWORK_MANAGER_H
#pragma once

#include "FrameworkApp.h"
#include "FreeRTOS.h"
#include "task.h"

#include "Network.h"
#include "NtpClient.h"

#define NETWORK_STACK_SIZE 1024
#define APPLICATION_STACK_SIZE 1024

class FrameworkManager {
public:
    FrameworkManager(FrameworkApp* app);
    void start();

private:
    FrameworkApp* app;
    TaskHandle_t networkTaskHandle;
    TaskHandle_t applicationTaskHandle;

    Network network;
    NTPClient ntpClient;

    static StaticTask_t xApplicationTaskBuffer;   
    static StaticTask_t xNetworkTaskBuffer;
    static StackType_t xNetworkStack[NETWORK_STACK_SIZE];
    static StackType_t xApplicationStack[APPLICATION_STACK_SIZE];

    static void network_task(void* params);
    static void app_task(void* params);
};

#endif
