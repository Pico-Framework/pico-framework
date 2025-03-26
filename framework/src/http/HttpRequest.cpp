/**
 * @file HttpRequest.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "HttpRequest.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include "HttpServer.h"
#include <cstring>
#include <cstdio>
#include <unordered_map>
#include "ff_stdio.h"
#include "lwip/sockets.h"
#include "utility.h"
#include "MultipartParser.h"

#ifdef TRACE_REQUEST
    #define DEBUG_PRINT(...) (std::cout << __VA_ARGS__ << std::endl)
#else
    #define DEBUG_PRINT(...) do {} while (0)
#endif

#define BUFFER_SIZE 1460  // this is the standard MTU size

Request::Request(const char* rawHeaders, const std::string& reqMethod, const std::string& reqPath)
    : method(reqMethod), path(reqPath) {

    parseHeaders(rawHeaders);
}

void Request::parseHeaders(const char* raw) {
    std::istringstream stream(raw);
    std::string line;

    while (std::getline(stream, line)) {

        // STOP when encountering an empty line (headers-body separator)
        if (line == "\r" || line.empty()) {
            break;
        }

        auto pos = line.find(":");
        if (pos != std::string::npos && pos + 1 < line.size()) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Remove unexpected characters
            key.erase(std::remove(key.begin(), key.end(), '\r'), key.end());
            value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
            key.erase(std::remove(key.begin(), key.end(), '"'), key.end());
            value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
            key = ::toLower(key);
            value.erase(0, value.find_first_not_of(" \t"));
            headers[key] = value;
        }
    }
}

int Request::receiveData(int clientSocket, char* buffer, int size) {
    
    size_t bytesReceived = lwip_recv(clientSocket, buffer, size-1, 0);
    if (bytesReceived < 0) {
        printf("Error receiving data from client: %d\n", bytesReceived);
        lwip_close(clientSocket);  // Close the client socket
        return -1;
    }

    printf("Received %zu bytes\n", bytesReceived);
    printf("Buffer: %.*s\n", bytesReceived, buffer);

    if (bytesReceived == 0) {
        printf("Client disconnected.\n");
        lwip_close(clientSocket);  // Close the client socket
        return -1;
    }

    buffer[bytesReceived] = '\0';  // Null-terminate the received data
    TRACE("Received %zu bytes\n", bytesReceived);
    TRACE("Buffer: %.*s\n", bytesReceived, buffer);
    return bytesReceived;
}

bool Request::getMethodAndPath(char* buffer, int clientSocket, char* method, char* path) {
    // Extract the HTTP method and path
    if (sscanf(buffer, "%s %s", method, path) != 2) {
        printf("Error parsing HTTP request method and path\n");
        lwip_close(clientSocket);
        return false;
    }
    return true;
}

Request Request::receive(int clientSocket) {
    
    char buffer[BUFFER_SIZE];  // Declare buffer size
    std::string body = "";  // Initialize empty body
    std::unordered_map<std::string, std::string> headers;  // Initialize empty headers
    char method[16] = {0};  // Initialize method buffer
    char path[BUFFER_SIZE] = {0};  // Initialize path buffer

    int bytesReceived = receiveData(clientSocket, (char*) &buffer, sizeof(buffer));
    if (bytesReceived == -1) {
        return Request("", "", "");  // Return empty Request on error
    }       
    if (!getMethodAndPath((char*)&buffer, clientSocket, (char*)&method, (char*)&path)){
        return Request("", "", "");  // Return empty Request on error
    }   

    // Indentify the raw headers - look for the end of the headers (a double CRLF "\r\n\r\n")
    size_t headerEnd = 0;
    while (headerEnd < bytesReceived) {
        if (buffer[headerEnd] == '\r' && buffer[headerEnd + 1] == '\n' &&
            buffer[headerEnd + 2] == '\r' && buffer[headerEnd + 3] == '\n') {
            headerEnd += 4;  // Move past the double CRLF that indicates the end of the headers
            break;
        }
        headerEnd++;
    }
    printf("HeaderEnd in receive: %zu\n", headerEnd);
    
    // Create the request which will parse the headers
    Request request(buffer, std::string(method), std::string(path));
 
    // Get headers from the Request object
    headers = request.getHeaders();  // Retrieve headers from the Request object

    TRACE("Parsed headers:\n");
    for (const auto& header : headers) {
        TRACE("%s: %s\n", header.first.c_str(), header.second.c_str());
    }

    // Get the content length from the headers
    int contentLength = request.getContentLength();
    printf("Content-Length: %zu\n", contentLength);

    // Handle the body only if contentLength is greater than 0
    if (contentLength > 0) {
        // First, handle any body data that might be in the first buffer
        size_t bodyReceived = bytesReceived - headerEnd;
        if (bodyReceived > 0) {
            body.append(buffer + headerEnd, bodyReceived);
        }
        request.setBody(body);  // Set the body in the Request object

        // Check for multipart data and delegate to handler if necessary
        if (request.isMultipart()) {
            TRACE("Multipart request detected\n");
            request.handle_multipart(clientSocket, request);
            return request;
        }
        printf("Non-multipart request detected\n");
        // For non-multipart data, set the body in the Request object

        // For non-multipart data, continue receiving the body data in chunks
        size_t bodyRemaining = contentLength - body.length();
        printf("Body remaining: %zu\n", bodyRemaining);
        while (bodyRemaining > 0) {
            size_t bytesToReceive = std::min(bodyRemaining, sizeof(buffer) - 1);  // Limit to buffer size
            size_t chunkReceived = lwip_recv(clientSocket, buffer, bytesToReceive, 0);

            if (chunkReceived <= 0) {
                printf("Error receiving body data, bytes received: %zu\n", chunkReceived);
                lwip_close(clientSocket);
                return Request("", "", "");  // Return empty Request on error
            }

            body.append(buffer, chunkReceived);  // Append received data to body
            bodyRemaining -= chunkReceived;  // Decrease remaining body size
        }
        printf("Body: %s\n", body.c_str());
    }
    printf("Request completed\n");
    return request;  // Return the constructed Request object
}

int Request::handle_multipart(int clientSocket, Request& req) {
    MultipartParser parser(clientSocket, req);
    return parser.handleMultipart() ? 0 : -1;
}

