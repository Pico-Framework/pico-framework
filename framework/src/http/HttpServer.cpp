/**
 * @file HttpServer.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "HttpServer.h" 
#include <lwip/sockets.h>
#include <lwip/netif.h>
#include <lwip/ip4_addr.h>
#include "lwip/stats.h"  // Required for `memp_stats`
#include "lwip/memp.h"  // Ensure lwIP memory pools are accessible
#include "lwip/tcp.h"

#include <cstring>   // memcpy, strcmp, etc.
#include <cstdio>    // printf
#include <cstdlib>   // for NULL
#include "FreeRTOS.h"
#include "task.h"
#include "utility.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef TRACE_ON
    //#define TRACE(format, ...) printf("TRACE: %s:%d: " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)
    #define TRACE(format, ...) printf(format, ##__VA_ARGS__)
#else
    #define TRACE(format, ...) /* do nothing */
#endif


// In http_server.hpp or at the top of http_server.cpp
#define BUFFER_SIZE 1024  // You can adjust the size based on expected data

// HttpServer constructor
HttpServer::HttpServer(int port, Router& router)
    : port(port), router(router) {
        TRACE("Router in http_server: %p\n", &router);
    };

StackType_t HttpServer::xStack[ HTTP_STACK_SIZE ];
StaticTask_t HttpServer::xTaskBuffer; 

bool HttpServer::start() {
    return xTaskCreateStatic(startServerTask, "HttpServer", HTTP_STACK_SIZE, this, TaskPrio_Mid, xStack, &xTaskBuffer);
}

void HttpServer::startServerTask(void* pvParameters) {
    HttpServer* server = static_cast<HttpServer*>(pvParameters);
    server->run();  // Run the actual server logic
    vTaskDelete(NULL); // Delete task when done
}

void HttpServer::run() {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    printf("Starting HTTP Server...\n");

    if (!initNetwork()) {
        return;  // If network initialization fails, return false
    }

    sock = initServerSocket();
    if (sock < 0) {
        return; // If socket initialization fails
    }

    while (true) {
        printf("Waiting for client connection...\n");

        int clientSocket = lwip_accept(sock, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            printf("Socket accept failed! lwIP error: %d\n", errno);
            vTaskDelay(pdMS_TO_TICKS(1000));  // Delay before retrying
            continue;
        }
        if (clientSocket > 1000) {  // Arbitrary high number to catch corruption
            printf("Warning: Unusually high client socket number detected: %d\n", clientSocket);
        }

        printf("Client connected on socket %d\n", clientSocket);

        // Directly handle the client within the same task
        handleClient(clientSocket);

        printf("Client request complete\n");
        printf("===== END CLIENT REQUEST ======\n\n");

        // Close the socket

        int ret = lwip_close(clientSocket);
        if (ret == 0) {
            TRACE("Successfully closed socket %d\n", clientSocket);
        } else {
            printf("Failed to close socket %d, errno: %d\n", clientSocket, errno);
        }
    }
}

// Initialize network: wait for IP address assignment via DHCP
bool HttpServer::initNetwork()
{
    struct netif* netif;
    TRACE("Waiting for DHCP lease...\n");

    // Wait for a valid network interface with an IP address assigned
    while (true) {
        netif = netif_list; // Get the first network interface
        if (netif && netif->ip_addr.addr != 0) {
            break;  // Exit when an IP address is assigned
        }
        vTaskDelay(pdMS_TO_TICKS(100));  // Wait for 100ms before checking again
    }

    // Log the assigned IP address once DHCP is successful
    TRACE("Assigned IP Address: %s\n", ip4addr_ntoa(&netif->ip_addr));

    return true;  // Network is up and IP address assigned
}

// Initialize the server socket
int HttpServer::initServerSocket()
{
    int s = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        printf("Error creating socket\n");
        return -1;
    }

    struct sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (lwip_bind(s, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Error binding socket\n");
        lwip_close(s);
        return -1;
    }

    if (lwip_listen(s, 1) < 0) {
        printf("Error listening on socket\n");
        lwip_close(s);
        return -1;
    }

    printf("HTTP server listening on port %d\n", port);
    return s;
}

// Handle HTTP request logic (with multipart support for /api/v1/upload)
void HttpServer::handleClient(int clientSocket) {

    TRACE("Router on handleClient entry: %p\n", (void*)&router);

    size_t contentLength = 0;
    char method[16] = {0};
    char path[BUFFER_SIZE] = {0}; 
    std::string body;
    std::unordered_map<std::string, std::string> headers;

    // Receive and process the request, including headers and body
    if (clientSocket < 0) {
        printf("Invalid client socket detected in handleClient: %d\n", clientSocket);
        return;
    }

    Request req = Request::receive(clientSocket);

    std::cout << "Request received: " << req.getMethod() << " " << req.getPath() << std::endl;
    std::cout << "Request content length: " << req.getContentLength() << std::endl;
    std::cout << "Request content type: " << req.getContentType() << std::endl;
    std::cout << "Request boundary: " << req.getBoundary() << std::endl;
    std::cout << "Request is multipart: " << req.isMultipart() << std::endl;
    std::cout << "Request header count: " << req.getHeaders().size() << std::endl;
    std::cout << "Request method: " << req.getMethod() << std::endl;
    std::cout << "Request path: " << req.getPath() << std::endl;
    std::cout << "Request headers: " << std::endl;
    std::cout << "Request start of body index: " << req.getHeaderEnd() << std::endl;
    std::unordered_map<std::string, std::string> headrs = req.getHeaders();
    for (const auto& headr : headrs) {
        std::cout << headr.first << ": " << headr.second << std::endl;
    }
    std::cout << "Request body: " << req.getBody() << std::endl;

    printf("\n===== HTTP CLIENT REQUEST =====\n");
    printf("Client request received: %s, path: %s\n", req.getMethod().c_str(), req.getPath().c_str());
    printf("Request body: %s\n", req.getBody().c_str());

    // Handle the HTTP request (forwarding to the router)

    bool ok = router.handleRequest(clientSocket, req.getMethod().c_str(), req.getPath().c_str(), req);
    printf("Request handled: %s\n", ok ? "true" : "false");

    if (!ok) {
        // If the router didn’t handle it, send 404
        const char* notFound =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<h1>404 Not Found</h1>";
        lwip_send(clientSocket, notFound, strlen(notFound), 0);
    }
    TRACE("Free Heap: %d bytes\n", xPortGetFreeHeapSize());
    TRACE("Min Ever Free Heap: %d bytes\n", xPortGetMinimumEverFreeHeapSize());
}

// Define this outside the method, ideally before any function that uses it
struct TaskParams {
    HttpServer* server;  // Pointer to the HttpServer instance
    int clientSocket;    // The client socket
};


// Function that will be used to handle each client in its own task
// void HttpServer::handleClientTask(void *pvParameters) {
//     TaskParams* params = (TaskParams*)pvParameters;
//     HttpServer* server = params->server;
//     int clientSocket = params->clientSocket;

//     printf("Handling client socket: %d\n", clientSocket);
    
//     server->handleClient(clientSocket);

//     printf("Closing client socket: %d\n", clientSocket);
//     lwip_close(clientSocket);  // Close the socket

//     vTaskDelete(NULL);
//     delete params;
// }



// // This method starts a FreeRTOS task to handle each client
// void HttpServer::startHandlingClient(int clientSocket) {
//     // Create a TaskParams object that holds both server and clientSocket
//     TaskParams* params = new TaskParams{this, clientSocket};
//     printf("Creating task to handle client socket: %d\n", clientSocket);
    
//     // Create a FreeRTOS task for each client
//     if(xTaskCreate([](void* pvParameters) {
//         // Extract the HttpServer object and client socket from parameters
//         TaskParams* params = (TaskParams*)pvParameters;
//         HttpServer* server = params->server;
//         int clientSocket = params->clientSocket;
//         printf("Handling client socket: %d\n", clientSocket);
        
//         // Handle the client

//         server->handleClient(clientSocket);

//         printf("Closing client socket: %d\n", clientSocket);

//         // Close the client socket after handling the request
//         lwip_shutdown(clientSocket, SHUT_RDWR);
//         lwip_close(clientSocket);

//         // Free the allocated memory for TaskParams
//         delete params;
//         TRACE("Client TaskParams deleted\n");

//         // Print before deleting task
//         printf("Deleting Client Task now...\n");

//         printSystemMemoryInfo();  // Print memory info before deleting task

//         // Delete the task after completion
//         vTaskDelete(NULL);

//     }, "HandleClient", 4096, params, tskIDLE_PRIORITY + 1, NULL) == pdPASS) {
//         printf("Client task created successfully\n");
//     } else {
//         printf("Failed to create task\n");
//         delete params;  // Free memory if task creation failed
//     }
// }

// This method now accepts and handles connections asynchronously
// void HttpServer::acceptClientConnections() {
//     struct sockaddr_in clientAddr;
//     socklen_t clientAddrLen = sizeof(clientAddr);

//     // Set a receive timeout for accept()
//     struct timeval timeout;
//     timeout.tv_sec = 5;   // 5 seconds timeout
//     timeout.tv_usec = 0;

//     int flags = lwip_fcntl(sock, F_GETFL, 0);
//     lwip_fcntl(sock, F_SETFL, flags | O_NONBLOCK);

//     while (true) {
//         printf("Waiting for client connection...\n");

//         static int retryCount = 0;
//         if (retryCount++ % 10 == 0) {
//             printf("Checking tcpip_thread: stack watermark = %d bytes\n", uxTaskGetStackHighWaterMark);
//         }
                
//         int clientSocket = -1;  // Explicitly initialize
    
//         clientSocket = lwip_accept(sock, (struct sockaddr*)&clientAddr, &clientAddrLen);
//         printf("Accept returned, client socket: %d\n", clientSocket);
    
//         // Detect lwIP returning an invalid socket number (e.g., 11)
//         if (clientSocket >= 10 || clientSocket < 0) {  // Assume valid sockets are <10
//             printf("Invalid socket detected, forcing retry...\n");
//             vTaskDelay(pdMS_TO_TICKS(1000));  // Prevent CPU spinning
//             continue;
//         }
    
//         printf("Client connected, creating task...\n");
//         startHandlingClient(clientSocket);
//         printf("Client handled\n");
//     }
    
// }


// void HttpServer::acceptClientConnections() {
//     struct sockaddr_in clientAddr;
//     socklen_t clientAddrLen = sizeof(clientAddr);

//     printf("Server is running in single-threaded mode.\n");

//     while (true) {
//         printf("\nWaiting for client connection...\n");

//         if (sock < 0) {
//             printf("ERROR: Listener socket is closed!\n");
//             return;
//         }

//         // Log FreeRTOS heap and TCP stack before accept
//         logSystemStats();

//         // Blocking accept call
//         int clientSocket = lwip_accept(sock, (struct sockaddr*)&clientAddr, &clientAddrLen);
//         printf("After lwip_accept(): clientSocket = %d\n", clientSocket);

//         if (clientSocket < 0) {
//             printf("lwip_accept() failed, error: %d\n", errno);
//             vTaskDelay(pdMS_TO_TICKS(1000));  // Small delay before retrying
//             continue;
//         }

//         printf("Client connected on socket %d\n", clientSocket);

//         // Log TCP state before handling
//         printTCPState();

//         // Handle the client request directly in this loop
//         printf("Router prior to handleClient call: %u\n", *(uint32_t*)&router);
//         handleClient(clientSocket);
//         printf("Client request complete\n");

//         // Ensure socket is properly closed
//         printf("Shutting down client socket %d...\n", clientSocket);
//         //lwip_shutdown(clientSocket, SHUT_RDWR);
//         lwip_close(clientSocket);
//         printf("Client socket %d closed\n", clientSocket);

//         // Log TCP state after handling
//         printTCPState();
//     }
// }

