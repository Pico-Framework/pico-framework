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

#include "http/HttpServer.h"
#include <lwip/sockets.h>
#include <lwip/netif.h>
#include <lwip/ip4_addr.h>
#include "lwip/stats.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "pico/stdlib.h"

#include "utility/utility.h"
#include "http/url_utils.h"
#include "framework/AppContext.h"
#include "time/TimeManager.h"
#include "network/Tcp.h"
#include "http/JsonResponse.h"
#include "events/EventManager.h"

// #define HTTP_SERVER_USE_TASK_PER_CLIENT  // ⚠️ NOT READY FOR PRODUCTION – Known instability with task-per-client mode
#ifdef HTTP_SERVER_USE_TASK_PER_CLIENT
#warning "⚠️ HTTP_SERVER_USE_TASK_PER_CLIENT is experimental and not yet production-ready. Use with caution."
#endif

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

    printf("[HttpServer] Starting HTTP Server on port %d\n", port);

    if (!initNetwork())
    {
        return;
    }

    Tcp* listener = initListener();
    if (!listener)
    {
        printf("[HttpServer] Failed to initialize listener\n");
        return;
    }

    AppContext::get<EventManager>()->postEvent({SystemNotification::HttpServerStarted});

    // Optional: store listener as a class member if needed later
    while (true)
    {
        TRACE("[HttpServer] Waiting for client connection...\n");
        //int64_t start = to_ms_since_boot(get_absolute_time());
        Tcp* conn = listener->accept();
        if (conn)
        {
            //int64_t accept_end = to_ms_since_boot(get_absolute_time());
            //printf("[HttpServer] Accept waited %lld ms\n", accept_end - start);
            QUIET_PRINTF("[HttpServer] Accepted client connection\n");
            //int64_t start = to_ms_since_boot(get_absolute_time());
            startHandlingClient(conn);
            //int64_t end = to_ms_since_boot(get_absolute_time());
            //printf("[HttpServer] Client handled in %lld ms\n", end - start);
            vTaskDelay(pdMS_TO_TICKS(10));
            

            QUIET_PRINTF("[HttpServer] Client connection handled\n");
            QUIET_PRINTF("===============================\n\n");
            #if !defined(HTTP_SERVER_USE_TASK_PER_CLIENT)
                delete conn;
            #endif
        }
        else
        {
            warning("[HttpServer] Failed to accept client connection\n");
            vTaskDelay(pdMS_TO_TICKS(10)); // Wait before retrying 
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
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    //TRACE("Assigned IP Address: %s\n", ip4addr_ntoa(&netif->ip_addr));
    return true;
}

/// @copydoc HttpServer::initServerSocket

Tcp* HttpServer::initListener()
{
    if (!listener.bindAndListen(port)) {
        return nullptr;
    }
    return &listener;
}

void HttpServer::startHandlingClient(Tcp* conn)
{
#ifdef HTTP_SERVER_USE_TASK_PER_CLIENT
    if (xSemaphoreTake(clientSemaphore, pdMS_TO_TICKS(100)) == pdPASS)
    {
        TaskParams *params = new TaskParams{this, conn};

        if (xTaskCreate(handleClientTask, "HttpClient", 4096, params, tskIDLE_PRIORITY + 1, NULL) == pdPASS)
        {
            TRACE("Client task created successfully");
        }
        else
        {
            TRACE("Failed to create client task");
            conn->close();
            delete params;
            xSemaphoreGive(clientSemaphore);
        }
    }
    else
    {
        TRACE("Max concurrent clients reached, closing connection");
        conn->close();
        // Optional: return a "503 Service Unavailable" if desired
    }
#else
    handleClient(conn);

#endif
}

/// @copydoc HttpServer::handleClient
void HttpServer::handleClient(Tcp* conn)
{
    int64_t start = to_ms_since_boot(get_absolute_time());
    int64_t lastActivity = start;
    const int64_t idleTimeoutMs = 5000; // 5 seconds idle timeout


    HttpRequest req = HttpRequest::receive(conn);
    if (req.getMethod().empty())
    {
        warning("[HttpServer] Empty HTTP method — client likely closed connection\n");
        return;
    }
    TRACE("HttpRequest received: %s, %s\n", req.getMethod().c_str(), req.getPath().c_str());
    TRACE("HttpRequest content length: %s\n", req.getContentLength());
    TRACE("HttpRequest content type: %s\n", req.getContentType());
    TRACE("HttpRequest boundary: %s\n", req.getBoundary());
    TRACE("HttpRequest is multipart: %s\n", (req.isMultipart() ? "true" : "false"));
    TRACE("HttpRequest header count: %d\n", req.getHeaders().size());
    TRACE("HttpRequest url: %s\n", req.getUri().c_str());
    TRACE("HttpRequest path: %s\n", req.getPath().c_str());
    TRACE("HttpRequest query: %s\n", req.getQuery().c_str());

    for (const auto& param : req.getQueryParams())
    {
        TRACE("HttpRequest query parameter %s : %s\n", param.first.c_str(), param.second.c_str());
    }

    for (const auto& param : req.getFormParams())
    {
        TRACE("HttpRequest form parameter %s : %s\n", param.first.c_str(), param.second.c_str());
    }

    for (const auto& cookie : req.getCookies())
    {
        TRACE("HttpRequest cookie %s : %s\n", cookie.first.c_str(), cookie.second.c_str());
    }

    if (req.getContentLength() > 0)
    {
        TRACE("HttpRequest body length: %d\n", req.getBody().length());
        TRACE("HttpRequest start of body index: %d\n", req.getHeaderEnd());
    }

    TRACE("HttpRequest headers:\n");
    for (const auto& headr : req.getHeaders())
    {
        TRACE("%s : %s\n", headr.first.c_str(), headr.second.c_str());
    }

    TRACE("HttpRequest body: %s\n", req.getBody().c_str());

    QUIET_PRINTF("\n===== HTTP CLIENT REQUEST =====\n");
    QUIET_PRINTF("[HttpServer] Client request received: %s, path: %s\n", req.getMethod().c_str(), req.getPath().c_str());

    HttpResponse res(conn);
    res.setHeader("Connection", "keep-alive"); // Add keep-alive header

    bool ok = router.handleRequest(req, res);
    TRACE("HttpRequest handled: %s\n", ok ? "true" : "false");

    if (!ok)
    {
        JsonResponse::sendError(res, 404, "NOT_FOUND", "route: " + std::string(req.getUri()));
    }

    lastActivity = to_ms_since_boot(get_absolute_time());

    if (req.getHeader("Connection") == "close")
    {
        printf("[HttpServer] Client requested Connection: close, closing connection\n");
        return;
    }

    if (to_ms_since_boot(get_absolute_time()) - lastActivity > idleTimeoutMs)
    {
        printf("[HttpServer] Idle timeout reached, closing connection\n");
        return;
    }

    conn->close();
    int64_t end = to_ms_since_boot(get_absolute_time());
    TRACE("[HttpServer] Client handled in %lld ms\n", end - start);
    QUIET_PRINTF("[HttpServer] Client connection handled\n");
    QUIET_PRINTF("===============================\n\n");
}


#ifdef HTTP_SERVER_USE_TASK_PER_CLIENT
void HttpServer::handleClientTask(void *pvParameters)
{
    TaskParams *params = static_cast<TaskParams *>(pvParameters);
    HttpServer *server = params->server;
    Tcp* tcp = params->tcp;

    TRACE("Handling client in task for socket %d\n", tcp->getSocketFd()); 

    server->handleClient(tcp);

    tcp->close();
    delete tcp;
    delete params;

    xSemaphoreGive(clientSemaphore);
    vTaskDelete(NULL);
}
#endif
