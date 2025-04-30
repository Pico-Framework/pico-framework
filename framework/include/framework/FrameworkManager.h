/**
 * @file FrameworkManager.h
 * @author Ian Archbell
 * @brief Orchestrates application startup and network initialization.
 *
 * The FrameworkManager is responsible for starting core tasks like networking,
 * waiting for connectivity, and notifying the application when it's ready to run.
 *
 * Designed to run early in the system lifecycle, it launches a static FreeRTOS task
 * for network bring-up and optionally for application initialization.
 *
 * @version 0.1
 * @date 2025-03-26
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#ifndef FRAMEWORK_MANAGER_H
#define FRAMEWORK_MANAGER_H
#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include "network/Network.h"
#include "framework/FrameworkController.h"

#define NETWORK_STACK_SIZE 2048 / sizeof(StackType_t)
#define APPLICATION_STACK_SIZE 1024 / sizeof(StackType_t)

class FrameworkApp;

/**
 * @brief Starts and coordinates core system services like networking and time sync.
 *
 * Responsible for launching the network task, waiting for connection, and notifying
 * the application via `FrameworkNotification::NetworkReady`.
 */
class FrameworkManager : public FrameworkController
{
public:
    /**
     * @brief Constructor.
     *
     * @param app Pointer to the application, which will be notified when the network is ready.
     */
    FrameworkManager(FrameworkApp *app, Router &router);


private:
    FrameworkApp *app;                  ///< Pointer to the application task
    TaskHandle_t networkTaskHandle;     ///< Handle for the network task
    TaskHandle_t applicationTaskHandle; ///< (Unused) optional handle for app task

    Network network; ///< Network management

    // Static task allocation buffers
    static StaticTask_t xApplicationTaskBuffer;
    static StaticTask_t xNetworkTaskBuffer;
    static StackType_t xNetworkStack[NETWORK_STACK_SIZE];
    static StackType_t xApplicationStack[APPLICATION_STACK_SIZE];

    /**
     * @brief Placeholder for an application-level task, if used.
     */
    static void app_task(void *params);

    /**
     * @brief Sets up debug tracing from configuration.
     *
     * Uses framework_config to set up tracing options, including file output if enabled.
     * This function should be called after the framework is initialized and
     * before any trace calls are made.
     *
     */
    void setupTraceFromConfig();

    /**
     * @brief Initializes the network and application tasks.
     *
     * This function is called to set up the network and application tasks
     * with the appropriate stack sizes and priorities.
     */
    void onStart();

    /**
     * @brief Polling function for the FrameworkManager.
     * 
     * This function checks the Wi-Fi connection status at regular intervals
     * and attempts to reconnect if the connection is lost.
     * 
     */
    void poll();

    /**
     * @brief Handles events posted to the FrameworkManager.
     * This function is called by the EventManager
     * when an event is posted to the FrameworkManager's event queue.
     */
    void onEvent(const Event& event);
};

#endif // FRAMEWORK_MANAGER_H
