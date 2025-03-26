// system_manager.hpp
#ifndef SYSTEM_MANAGER_HPP
#define SYSTEM_MANAGER_HPP

#include "network.h"
#include "ntpClient.h"
#include "app.h"

#define NETWORK_STACK_SIZE 512

class SystemManager {
public:
    SystemManager(int port);
    void start();

private:
    static void network_task(void* params);
    static void app_task(void* params);
    App app;
    Network network;
    NTPClient ntpClient;
    TaskHandle_t networkTaskHandle;
    TaskHandle_t applicationTaskHandle;
    static StackType_t xNetworkStack[ NETWORK_STACK_SIZE ];
    static StaticTask_t xNetworkTaskBuffer;
    static StackType_t xApplicationStack[ APPLICATION_STACK_SIZE ];
    static StaticTask_t xApplicationTaskBuffer;   
   

};

#endif // SYSTEM_MANAGER_HPP