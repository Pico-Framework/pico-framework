/**
 * @file HttpServer.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#pragma once

#include <string>
#include "Router.h"
#include <unordered_map>

#define HTTP_STACK_SIZE 8*1024
class HttpServer
{
public:
    // Constructor
    HttpServer(int port, Router& router);  // Pass Router as a reference
    
    // Start the HTTP server: Initializes the network, routes, and socket
    bool start();

    // Initialize the network: Wait for DHCP lease or use static IP
    bool initNetwork();

    // Main server loop to accept and handle incoming client requests
    void run();

    static void startServerTask(void* pvParameters);

    // Create and bind the server socket
    int initServerSocket();

    // Handle incoming client requests
    void handleClient(int clientSocket);

    int getSocket() const { return sock; }

    // Receive HTTP request and parse headers
    Request receiveRequest(int clientSocket, char* method, char* path, std::string& body, size_t& contentLength, std::unordered_map<std::string, std::string>& headers);

    Router& getRouter() { return router; }
    static void handleClientTask(void *pvParameters);
    void startHandlingClient(int clientSocket);
    void acceptClientConnections();


private:
    int port;         // Port on which the server will listen
    int sock;         // Socket descriptor
    Router& router;    // Router to handle the requests
    static constexpr int BUFFER_SIZE = 1024;
    static constexpr int BOUNDARY_MAX_LEN = 128;

    static StackType_t xStack[ HTTP_STACK_SIZE ];
    static StaticTask_t xTaskBuffer;   
};

#endif // HTTP_SERVER_H
