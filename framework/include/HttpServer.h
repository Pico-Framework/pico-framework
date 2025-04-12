/**
 * @file HttpServer.h
 * @author Ian Archbell
 * @brief HTTP Server class that listens for connections and dispatches requests to a Router.
 * @version 0.1
 * @date 2025-03-26
 * 
 * @license MIT License
 * 
 * @copyright Copyright (c) 2025
 */

 #ifndef HTTP_SERVER_H
 #define HTTP_SERVER_H
 #pragma once
 
 #include <string>
 #include <unordered_map>
 #include "Router.h"
 #include "FreeRTOS.h"
 #include "task.h"
 
 #define HTTP_STACK_SIZE 8*1024 / sizeof(StackType_t) // Stack size for FreeRTOS task
 
 /**
  * @brief HTTP Server that listens for incoming connections and dispatches requests.
  */
 class HttpServer
 {
 public:
     /**
      * @brief Construct a new HttpServer instance.
      * @param port Port number to listen on.
      * @param router Reference to the Router for handling requests.
      */
     HttpServer(int port, Router& router);
 
     /**
      * @brief Start the HTTP server as a FreeRTOS task.
      * @return true if task created successfully.
      */
     bool start();
 
     /**
      * @brief Initialize the network stack (wait for DHCP or static IP).
      * @return true if successful.
      */
     bool initNetwork();
 
     /**
      * @brief Main server loop: initializes, binds, and begins accepting connections.
      */
     void run();
 
     /**
      * @brief Launch the HTTP server task (used by FreeRTOS).
      * @param pvParameters Pointer to `HttpServer` instance.
      */
     static void startServerTask(void* pvParameters);
 
     /**
      * @brief Create, bind, and listen on the server socket.
      * @return Socket file descriptor, or -1 on failure.
      */
     int initServerSocket();
 
     /**
      * @brief Accept a client connection and handle it directly (not task-based).
      * @param clientSocket Client's socket file descriptor.
      */
     void handleClient(int clientSocket);
 
     /**
      * @brief Spawn a task to handle the client connection.
      * @param clientSocket Client's socket file descriptor.
      */
     void startHandlingClient(int clientSocket);
 
     /**
      * @brief Return the server socket descriptor.
      * @return Socket file descriptor.
      */
     int getSocket() const { return sock; }
 
     /**
      * @brief Receive an HTTP request and parse key components.
      * @param clientSocket Socket descriptor for the client.
      * @param method Output buffer for method.
      * @param path Output buffer for path.
      * @param body Output body content.
      * @param contentLength Output content length.
      * @param headers Output map of parsed headers.
      * @return Parsed HttpRequest object.
      */
     HttpRequest receiveRequest(int clientSocket, char* method, char* path, std::string& body,
                            size_t& contentLength, std::unordered_map<std::string, std::string>& headers);
 
     /**
      * @brief Return the router associated with the server.
      * @return Router reference.
      */
     Router& getRouter() { return router; }
 
     /**
      * @brief Handle client logic inside a FreeRTOS task.
      * @param pvParameters Pointer to TaskParams with server/clientSocket.
      */
     static void handleClientTask(void* pvParameters);
 
     /**
      * @brief Accept client connections in a blocking loop and spawn handlers.
      */
     void acceptClientConnections();
 
 private:
     int port;             ///< Port number to listen on.
     int sock;             ///< Server socket file descriptor.
     Router& router;       ///< Reference to router for dispatching requests.
 
     static constexpr int BUFFER_SIZE = 1024;
     static constexpr int BOUNDARY_MAX_LEN = 128;
 
     static StackType_t xStack[HTTP_STACK_SIZE];   ///< Stack for static FreeRTOS task.
     static StaticTask_t xTaskBuffer;              ///< Task control block buffer.
 };
 
 #endif // HTTP_SERVER_H
 