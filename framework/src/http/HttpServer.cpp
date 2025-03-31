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
 //#include "TcpConnectionSocket.h"
 
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
 struct TaskParams {
    HttpServer* server;   // Pointer to the HttpServer instance
    int clientSocket;     // The client socket
};


 // ----------------------------------------------------------------------------
 // Constructor and Task Entry
 // ----------------------------------------------------------------------------
 
 /// @copydoc HttpServer::HttpServer
 HttpServer::HttpServer(int port, Router& router)
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
 void HttpServer::startServerTask(void* pvParameters)
 {
     HttpServer* server = static_cast<HttpServer*>(pvParameters);
     server->run();
     vTaskDelete(NULL);
 }
 
 // ----------------------------------------------------------------------------
 // Server Core
 // ----------------------------------------------------------------------------
 
 /// @copydoc HttpServer::run
 void HttpServer::run()
 {
     printf("Starting HTTP Server...\n");
 
     if (!initNetwork()) {
         return;
     }
 
     sock = initServerSocket();
     if (sock < 0) {
         return;
     }
 
     acceptClientConnections();
 }
 
 /// @copydoc HttpServer::initNetwork
 bool HttpServer::initNetwork()
 {
     struct netif* netif;
 
     TRACE("Waiting for DHCP lease...\n");
 
     while (true) {
         netif = netif_list;
         if (netif && netif->ip_addr.addr != 0) {
             break;
         }
         vTaskDelay(pdMS_TO_TICKS(100));
     }
 
     TRACE("Assigned IP Address: %s\n", ip4addr_ntoa(&netif->ip_addr));
     return true;
 }
 
 /// @copydoc HttpServer::initServerSocket
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
 
 // ----------------------------------------------------------------------------
 // Connection Handling
 // ----------------------------------------------------------------------------
 
 /// @copydoc HttpServer::acceptClientConnections
 void HttpServer::acceptClientConnections()
 {
     struct sockaddr_in clientAddr;
     socklen_t clientAddrLen = sizeof(clientAddr);
 
     while (true) {
         printf("Waiting for client connection...\n");
 
         int clientSocket = lwip_accept(sock, (struct sockaddr*)&clientAddr, &clientAddrLen);
         if (clientSocket < 0) {
             printf("lwip_accept failed, error: %d\n", errno);
             vTaskDelay(pdMS_TO_TICKS(1000));
             continue;
         }
 
         printf("Client connected on socket %d, spawning task...\n", clientSocket);
         startHandlingClient(clientSocket);
     }
 }
 
 /// @copydoc HttpServer::startHandlingClient
 void HttpServer::startHandlingClient(int clientSocket)
 {
    if (xSemaphoreTake(clientSemaphore, pdMS_TO_TICKS(100)) == pdPASS)
    {
        TaskParams* params = new TaskParams{this, clientSocket};
    
        if (xTaskCreate(handleClientTask, "HttpClient", 4096, params, tskIDLE_PRIORITY + 1, NULL) == pdPASS) {
            printf("Client task created successfully for socket %d\n", clientSocket);
        } else {
            printf("Failed to create client task for socket %d\n", clientSocket);
            lwip_close(clientSocket);
            delete params;
        }
    }
    else
    {
        printf("Max concurrent clients reached, closing socket %d\n", clientSocket);
        lwip_close(clientSocket);
        // Optionally, you could send a response to the client indicating the server is busy.
        // semaphore will be released in handleClientTask
    }
 }
 
 /// @copydoc HttpServer::handleClient
 void HttpServer::handleClient(int clientSocket)
 {
     if (clientSocket < 0) {
         printf("Invalid client socket detected in handleClient: %d\n", clientSocket);
         return;
     }
 
     printf("Handling client socket: %d\n", clientSocket);
 
     Request req = Request::receive(clientSocket);
     std::string clientIp = getClientIpFromSocket(clientSocket);
     req.setClientIp(clientIp);
 
     std::cout << "Client IP: " << clientIp << std::endl;
     std::cout << "Request received: " << req.getMethod() << " " << req.getPath() << std::endl;
     std::cout << "Request content length: " << req.getContentLength() << std::endl;
     std::cout << "Request content type: " << req.getContentType() << std::endl;
     std::cout << "Request boundary: " << req.getBoundary() << std::endl;
     std::cout << "Request is multipart: " << (req.isMultipart() ? "true" : "false") << std::endl;
     std::cout << "Request header count: " << req.getHeaders().size() << std::endl;
     std::cout << "Request method (lowercase): " << toLower(req.getMethod()) << std::endl;
     std::cout << "Request url: " << req.getUrl() << std::endl;
     std::cout << "Request path: " << req.getPath() << std::endl;
     std::cout << "Request query: " << req.getQuery() << std::endl;
 
     for (const auto& param : req.getQueryParams()) {
         std::cout << param.first << ": " << param.second << std::endl;
     }
 
     for (const auto& param : req.getFormParams()) {
         std::cout << param.first << ": " << param.second << std::endl;
     }
 
     for (const auto& cookie : req.getCookies()) {
         std::cout << cookie.first << ": " << cookie.second << std::endl;
     }
 
     if (req.getContentLength() > 0) {
         std::cout << "Request body length: " << req.getBody().length() << std::endl;
         std::cout << "Request start of body index: " << req.getHeaderEnd() << std::endl;
     }
 
     for (const auto& headr : req.getHeaders()) {
         std::cout << headr.first << ": " << headr.second << std::endl;
     }
 
     std::cout << "Request body: " << req.getBody() << std::endl;
 
     printf("\n===== HTTP CLIENT REQUEST =====\n");
     printf("Client request received: %s, path: %s\n", req.getMethod().c_str(), req.getPath().c_str());
     printf("Request body: %s\n", req.getBody().c_str());
 
     bool ok = router.handleRequest(clientSocket, req.getMethod().c_str(), req.getPath().c_str(), req);
     printf("Request handled: %s\n", ok ? "true" : "false");
 
     if (!ok) {
         const char* notFound =
             "HTTP/1.1 404 Not Found\r\n"
             "Content-Type: text/html\r\n\r\n"
             "<h1>404 Not Found</h1>";
         lwip_send(clientSocket, notFound, strlen(notFound), 0);
     }
 
     TRACE("Free Heap: %d bytes\n", xPortGetFreeHeapSize());
     TRACE("Min Ever Free Heap: %d bytes\n", xPortGetMinimumEverFreeHeapSize());
 }
 
 /// @copydoc HttpServer::handleClientTask
 void HttpServer::handleClientTask(void* pvParameters)
 {
     TaskParams* params = static_cast<TaskParams*>(pvParameters);
     HttpServer* server = params->server;
     int clientSocket = params->clientSocket;
 
     printf("Handling client in task for socket %d\n", clientSocket);
     server->handleClient(clientSocket);
 
     lwip_close(clientSocket);
 
     delete params;
     printf("Client socket %d closed and task deleted\n", clientSocket);
     xSemaphoreGive(clientSemaphore); // Release the semaphore for the next client
     vTaskDelete(NULL);
 }
 