/**
 * @file HttpServer.cpp
 * @brief HTTP Server implementation with per-client task handling.
 * 
 * This version spawns a new FreeRTOS task for each accepted client.
 */

 #include "HttpServer.h" 
 #include <lwip/sockets.h>
 #include <lwip/netif.h>
 #include <lwip/ip4_addr.h>
 #include "lwip/stats.h"      // Required for memp_stats
 #include "lwip/memp.h"       // Ensure lwIP memory pools are accessible
 #include "lwip/tcp.h"
 
 #include <cstring>   // memcpy, strcmp, etc.
 #include <cstdio>    // printf
 #include <cstdlib>   // for NULL
 #include "FreeRTOS.h"
 #include "task.h"
 #include "utility.h"
 #include "url_utils.h"
 #include "TcpConnectionSocket.h"
 
 #define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
 
 #ifdef TRACE_ON
     #define TRACE(format, ...) printf(format, ##__VA_ARGS__)
 #else
     #define TRACE(format, ...) /* do nothing */
 #endif
 
 #define BUFFER_SIZE 1024  // Adjust the buffer size based on expected data
 
 // Static task buffer and stack for the main server task
 StackType_t HttpServer::xStack[ HTTP_STACK_SIZE ];
 StaticTask_t HttpServer::xTaskBuffer; 
 
 // Structure to hold parameters for client tasks
 struct TaskParams {
     HttpServer* server;   // Pointer to the HttpServer instance
     int clientSocket;     // The client socket
 };
 
 bool HttpServer::start() {
     return xTaskCreateStatic(startServerTask, "HttpServer", HTTP_STACK_SIZE, this, TaskPrio_Mid, xStack, &xTaskBuffer);
 }
 
 void HttpServer::startServerTask(void* pvParameters) {
     HttpServer* server = static_cast<HttpServer*>(pvParameters);
     server->run();  // Run the server logic (includes the accept loop)
     vTaskDelete(NULL); // Delete the task when done
 }
 
 void HttpServer::run() {
     printf("Starting HTTP Server...\n");
 
     if (!initNetwork()) {
         return;  // Network initialization failed
     }
 
     sock = initServerSocket();
     if (sock < 0) {
         return;  // Socket initialization failed
     }
 
     // Start accepting client connections asynchronously
     acceptClientConnections();
 }
 
 bool HttpServer::initNetwork() {
     struct netif* netif;
     TRACE("Waiting for DHCP lease...\n");
 
     // Wait until a network interface gets an IP address assigned via DHCP
     while (true) {
         netif = netif_list; // Get the first network interface
         if (netif && netif->ip_addr.addr != 0) {
             break;  // IP address assigned
         }
         vTaskDelay(pdMS_TO_TICKS(100));
     }
 
     TRACE("Assigned IP Address: %s\n", ip4addr_ntoa(&netif->ip_addr));
     return true;
 }
 
 int HttpServer::initServerSocket() {
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
 
 void HttpServer::acceptClientConnections() {
     struct sockaddr_in clientAddr;
     socklen_t clientAddrLen = sizeof(clientAddr);
 
     while (true) {
         printf("Waiting for client connection...\n");
 
         int clientSocket = lwip_accept(sock, (struct sockaddr*)&clientAddr, &clientAddrLen);
         if (clientSocket < 0) {
             printf("lwip_accept failed, error: %d\n", errno);
             vTaskDelay(pdMS_TO_TICKS(1000));  // Delay before retrying
             continue;
         }
 
         printf("Client connected on socket %d, spawning task...\n", clientSocket);
         startHandlingClient(clientSocket);
     }
 }
 
 void HttpServer::startHandlingClient(int clientSocket) {
     // Allocate a TaskParams structure for the new client task
     TaskParams* params = new TaskParams{this, clientSocket};
 
     // Create a new FreeRTOS task to handle the client
     if (xTaskCreate(handleClientTask, "HttpClient", 4096, params, tskIDLE_PRIORITY + 1, NULL) == pdPASS) {
          printf("Client task created successfully for socket %d\n", clientSocket);
     } else {
          printf("Failed to create client task for socket %d\n", clientSocket);
          lwip_close(clientSocket);
          delete params;
     }
 }
 
 void HttpServer::handleClient(int clientSocket) {
     // Process the HTTP request using your existing logic
 
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
 
     // Forward the request to the router for handling
     bool ok = router.handleRequest(clientSocket, req.getMethod().c_str(), req.getPath().c_str(), req);
     printf("Request handled: %s\n", ok ? "true" : "false");
 
     if (!ok) {
         // If the router did not handle the request, send a 404 response
         const char* notFound =
             "HTTP/1.1 404 Not Found\r\n"
             "Content-Type: text/html\r\n\r\n"
             "<h1>404 Not Found</h1>";
         lwip_send(clientSocket, notFound, strlen(notFound), 0);
     }
     TRACE("Free Heap: %d bytes\n", xPortGetFreeHeapSize());
     TRACE("Min Ever Free Heap: %d bytes\n", xPortGetMinimumEverFreeHeapSize());
 }
 
 // Static task function to handle each client connection in its own task
 void HttpServer::handleClientTask(void* pvParameters) {
     TaskParams* params = static_cast<TaskParams*>(pvParameters);
     HttpServer* server = params->server;
     int clientSocket = params->clientSocket;
 
     printf("Handling client in task for socket %d\n", clientSocket);
     server->handleClient(clientSocket);
 
     // Shutdown and close the client socket after processing
     lwip_shutdown(clientSocket, SHUT_RDWR);
     lwip_close(clientSocket);
 
     delete params;  // Clean up allocated parameters
     printf("Client socket %d closed and task deleted\n", clientSocket);
     vTaskDelete(NULL);
 }
 