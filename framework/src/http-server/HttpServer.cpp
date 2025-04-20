/**
 * @file HttpServer.cpp
 * @author Ian Archbell
 * @brief HTTP Server implementation with per-client task handling.
 *
 * Part of the PicoFramework HTTP server.
 * This module handles incoming HTTP requests, spawns a new FreeRTOS task for each
 * accepted client, and processes the requests using a router.
 * It uses the lwIP stack for network communication and FreeRTOS for task management.
 * The server can handle multiple clients concurrently, limited by the MAX_CONCURRENT_CLIENTS.
 *
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 *
 */

#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"
TRACE_INIT(HttpServer)

#include "HttpServer.h"
#include <lwip/sockets.h>
#include <lwip/netif.h>
#include <lwip/ip4_addr.h>
#include "lwip/stats.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "utility.h"
#include "url_utils.h"
#include "AppContext.h"
#include "TimeManager.h"
#include "Tcp.h"
#include "JsonResponse.h"

#define BUFFER_SIZE 1024

static constexpr int MAX_CONCURRENT_CLIENTS = 1;
SemaphoreHandle_t clientSemaphore;

StackType_t HttpServer::xStack[HTTP_STACK_SIZE];
StaticTask_t HttpServer::xTaskBuffer;

// ----------------------------------------------------------------------------
// Task Context Struct
// ----------------------------------------------------------------------------

/**
 * @brief Parameters passed to the per-client handler task.
 */
struct TaskParams
{
    HttpServer *server; // Pointer to the HttpServer instance
    Tcp *tcp;   // The client TCP connection
};

// ----------------------------------------------------------------------------
// Constructor and Task Entry
// ----------------------------------------------------------------------------

/// @copydoc HttpServer::HttpServer
HttpServer::HttpServer(int port, Router &router)
    : port(port), router(router)
{
}

/// @copydoc HttpServer::start
bool HttpServer::start()
{
    clientSemaphore = xSemaphoreCreateCounting(MAX_CONCURRENT_CLIENTS, MAX_CONCURRENT_CLIENTS);
    return xTaskCreateStatic(startServerTask, "HttpServer", HTTP_STACK_SIZE, this, TaskPrio_Mid, xStack, &xTaskBuffer);
}

/// @copydoc HttpServer::startServerTask
void HttpServer::startServerTask(void *pvParameters)
{
    HttpServer *server = static_cast<HttpServer *>(pvParameters);
    server->run();
    vTaskDelete(NULL);
}

// ----------------------------------------------------------------------------
// Server Core
// ----------------------------------------------------------------------------

/// @copydoc HttpServer::run
void HttpServer::run()
{
    printf("[Http Server] Starting HTTP Server...\n");

    if (!initNetwork())
    {
        return;
    }

    AppContext::getInstance().getService<TimeManager>()->detectAndApplyTimezone();
    //TRACE("Timezone applied\n");

    Tcp* listener = initListener();
    if (!listener)
    {
        printf("[HttpServer] Failed to initialize listener\n");
        return;
    }

    // Optional: store listener as a class member if needed later
    while (true)
    {
        printf("[HttpServer] Waiting for client connection...\n");
        Tcp* conn = listener->accept();
        if (conn)
        {
            printf("[HttpServer] Accepted client connection\n");
            startHandlingClient(conn);
            vTaskDelay(pdMS_TO_TICKS(100));
            printf("[HttpServer] Client connection handled\n");
            printf("==============================\n\n");
            delete conn;
        }
    }

    // Note: we never reach here in this model
    delete listener;
}


/// @copydoc HttpServer::initNetwork
bool HttpServer::initNetwork()
{
    struct netif *netif;

    //TRACE("Waiting for DHCP lease...\n");

    while (true)
    {
        netif = netif_list;
        if (netif && netif->ip_addr.addr != 0)
        {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    //TRACE("Assigned IP Address: %s\n", ip4addr_ntoa(&netif->ip_addr));
    return true;
}

/// @copydoc HttpServer::initServerSocket

Tcp* HttpServer::initListener()
{
    Tcp* listener = new Tcp();
    if (!listener->bindAndListen(port)) {
        delete listener;
        return nullptr;
    }
    return listener;
}

// ----------------------------------------------------------------------------
// Connection Handling
// ----------------------------------------------------------------------------

/// @copydoc HttpServer::acceptClientConnections
// void HttpServer::acceptClientConnections()
// {
//     Tcp* listener = initListener();
//     if (!listener) {
//         printf("[HttpServer] Failed to initialize listener\n");
//         return;
//     }

//     while (true)
//     {
//         printf("[HttpServer] Waiting for client connection...\n");
//         Tcp* conn = listener->accept();
//         if (!conn)
//         {
//             // Accept failed or no client — skip
//             continue;
//         }
//         printf("[HttpServer] Accepted client connection\n");
//         startHandlingClient(conn);
//         printf("[HttpServer] Client connection handled\n");
//         delete conn;
//     }

//     // Unreachable in current model, but if you ever shut down:
//     delete listener;
// }

void HttpServer::startHandlingClient(Tcp* conn)
{
    // Direct call for now — FreeRTOS task dispatch can be re-enabled later
    handleClient(conn);
}

// this is not converted to use tcp class
/// @copydoc HttpServer::startHandlingClient
// void HttpServer::startHandlingClient(Tcp* conn)
// {
//     if (xSemaphoreTake(clientSemaphore, pdMS_TO_TICKS(100)) == pdPASS)
//     {
//         TaskParams *params = new TaskParams{this, clientSocket};

//         if (xTaskCreate(handleClientTask, "HttpClient", 4096, params, tskIDLE_PRIORITY + 1, NULL) == pdPASS)
//         {
//             printf("Client task created successfully for socket %d\n", clientSocket);
//         }
//         else
//         {
//             printf("Failed to create client task for socket %d\n", clientSocket);
//             conn->close();
//             delete params;
//         }
//     }
//     else
//     {
//         printf("Max concurrent clients reached, closing socket %d\n", clientSocket);
//         lwip_close(clientSocket);
//         // Optionally, you could send a response to the client indicating the server is busy.
//         // semaphore will be released in handleClientTask
//     }
// }

/// @copydoc HttpServer::handleClient
void HttpServer::handleClient(Tcp* conn)
{

    HttpRequest req = HttpRequest::receive(conn);

    TRACE("HttpRequest received: %s, %s\n",req.getMethod().c_str(), req.getPath().c_str());
    TRACE("HttpRequest content length: %s\n", req.getContentLength());
    TRACE("HttpRequest content type: %s\n", req.getContentType());
    TRACE("HttpRequest boundary: %s\n", req.getBoundary());
    TRACE("HttpRequest is multipart: %s\n", (req.isMultipart() ? "true" : "false"));
    TRACE("HttpRequest header count: %d\n", req.getHeaders().size());;
    TRACE("HttpRequest url: %s\n", req.getUri().c_str());
    TRACE("HttpRequest path: %s\n", req.getPath().c_str());
    TRACE("HttpRequest query: %s\n", req.getQuery().c_str());

    for (const auto &param : req.getQueryParams())
    {
        TRACE("HttpRequest query parameter %s : %s\n", param.first.c_str(), param.second.c_str()); 
    }

    for (const auto &param : req.getFormParams())
    {
        TRACE("HttpRequest form parameter %s : %s\n", param.first.c_str(), param.second.c_str()); 
    }

    for (const auto &cookie : req.getCookies())
    {
        TRACE("HttpRequest cookie %s : %s\n", cookie.first.c_str(), cookie.second.c_str()); 
    }

    if (req.getContentLength() > 0)
    {
        TRACE("HttpRequest body length: %d\n", req.getBody().length());
        TRACE("HttpRequest start of body index: %d\n", req.getHeaderEnd());
    }
    
    TRACE("HttpRequest headers:\n");
    for (const auto &headr : req.getHeaders())    
    {
        TRACE("%s : %s\n", headr.first.c_str(), headr.second.c_str());
    }

    TRACE("HttpRequest body: %s\n", req.getBody().c_str()); 

    printf("\n===== HTTP CLIENT REQUEST =====\n");
    printf("[HttpServer] Client request received: %s, path: %s\n", req.getMethod().c_str(), req.getPath().c_str());

    HttpResponse res(conn);
    bool ok = router.handleRequest(req, res);
    TRACE("HttpRequest handled: %s\n", ok ? "true" : "false");

    if (!ok)
    {
        JsonResponse::sendError(res, 404, "NOT_FOUND", "route: " + std::string(req.getUri()));
    }

}

/// @copydoc HttpServer::handleClientTask
// void HttpServer::handleClientTask(void *pvParameters)
// {
//     TaskParams *params = static_cast<TaskParams *>(pvParameters);
//     HttpServer *server = params->server;
//     Tcp* tcp = params->tcp;

//     printf("Handling client in task for socket %d\n", tcp->getSocketFd());  
//     server->handleClient(tcp);

//     tcp->close();

//     delete params;
//     printf("Client socket %d closed and task deleted\n", tcp->getSocketFd());
//     xSemaphoreGive(clientSemaphore); // Release the semaphore for the next client
//     vTaskDelete(NULL);
// }
