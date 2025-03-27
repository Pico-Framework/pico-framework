/**
 * @file MultipartParser.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "MultipartParser.h"
#include "ff_stdio.h"
#include "lwip/sockets.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include "AppContext.h"

#define BUFFER_SIZE 1460

MultipartParser::MultipartParser(int clientSocket, const Request& request)
    : clientSocket(clientSocket),
      request(request) {
    // Initialize the boundary from the request
    boundary = request.getBoundary();
}

// Main function to handle multipart processing
bool MultipartParser::handleMultipart() {
    if (boundary.empty()) {
        sendHttpResponse(400, "Missing boundary");
        return false;
    }
    printf("Handing multipart data with boundary: %s\n", boundary.c_str());
    char buffer[BUFFER_SIZE];
    int received;

    // Process incoming chunks
    while ((received = lwip_recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        std::string chunk(buffer, received);
        if (!handleChunk(chunk)) return false;
    }

    return true;
}

// Extract filename from content-disposition header
bool MultipartParser::extractFilename(const std::string& contentDisposition) {
    size_t filenamePos = contentDisposition.find("filename=\"");
    if (filenamePos != std::string::npos) {
        filename = contentDisposition.substr(filenamePos + 10); // skip 'filename="'
        filename = filename.substr(0, filename.find("\""));
        printf("Extracted filename: %s\n", filename.c_str());
        // Check if file already exists
        if (file_exists(filename.c_str())) {
            printf("File already exists: %s\n", filename.c_str());
            return false;
        }
        return true;
    }
    printf("Filename not found in content-disposition\n");
    return false;
}

// Process and save file chunks
bool MultipartParser::processFileData(const std::string& fileData) {
    return appendFileToSD(filename.c_str(), fileData.c_str(), fileData.size()) == 0;
}

// Handles incoming chunks and state transitions
bool MultipartParser::handleChunk(std::string& chunkData) {
    int dataStartPos;

    if (chunkData.empty()) {
        sendHttpResponse(400, "Empty chunk data");
        return false;
    }
    // Handle different states of the upload
    while (true) {
        switch (currentState) {
            case SEARCHING_FOR_BOUNDARY:
                // Check if boundary exists in headers
                if (request.getBoundary().empty()) {
                    sendHttpResponse(400, "Content-Type header not found");
                    printf("Boundary not found in headers\n");
                    lwip_close(clientSocket);
                    return false;
                } else {   
                    std::string initial_boundary = "--" + request.getBoundary();
                    size_t boundary_pos = chunkData.find(initial_boundary);                
                    if (boundary_pos != std::string::npos){
                        printf("Initial boundary found\n");
                        chunkData = chunkData.substr(boundary_pos + initial_boundary.size());
                        currentState = FOUND_BOUNDARY;
                    }
                    else{
                        currentState = SEARCHING_FOR_BOUNDARY;
                        return false;
                    }   
                }
                break;

                case FOUND_BOUNDARY:
                    
                    if (!extractFilename(chunkData)) {
                        sendHttpResponse(409, "File already exists");
                        currentState = SEARCHING_FOR_BOUNDARY;
                        return false;
                    }

                    dataStartPos = findDataStart(chunkData);
                    if (dataStartPos == -1) {
                        currentState = SEARCHING_FOR_BOUNDARY;
                        return false;
                    }
                    chunkData = chunkData.substr(dataStartPos);
                    currentState = FOUND_DATA_START;
                    break;

            case FOUND_DATA_START: {
                // Handle the final boundary (upload completion)
                if (handleFinalBoundary(chunkData)) {
                    if (!processFileData(chunkData)) return false;
                    sendHttpResponse(200, "Upload complete");
                    lwip_close(clientSocket);
                    currentState = SEARCHING_FOR_BOUNDARY;
                    printf("File upload completed successfully\n");
                    return true;
                }

                // Process the file data
                if (!processFileData(chunkData)) return false;
                return true;
            }
        }
    }

    return true;
}

// Finds the start of the file data within the request body
int MultipartParser::findDataStart(std::string& currentData) {
    size_t pos = currentData.find("\r\n\r\n");
    return (pos != std::string::npos) ? pos + 4 : -1;
}

// Handles the final boundary and trims extra data
bool MultipartParser::handleFinalBoundary(std::string& fileData) {
    std::string finalBoundary = "\r\n--" + boundary + "--";
    size_t pos = fileData.find(finalBoundary);
    if (pos != std::string::npos) {
        fileData = fileData.substr(0, pos);
        return true;
    }
    return false;
}

// Save file chunks to SD
// int MultipartParser::appendFileToSD(const char* filename, const char* data, size_t size) {
//     printf("Appending data to SD: %s, size: %zu\n", filename, size);
//     char filepath[128];
//     snprintf(filepath, sizeof(filepath), "/sd0/%s", filename);

//     FF_FILE* file = ff_fopen(filepath, "a");
//     if (!file) return -1;

//     size_t written = ff_fwrite(data, 1, size, file);
//     ff_fclose(file);

//     return (written == size) ? 0 : -1;
// }

int MultipartParser::appendFileToSD(const char* filename, const char* data, size_t size) {
    printf("Appending data to SD: %s, size: %zu\n", filename, size);

    std::string filepath = std::string("/") + filename;  // No /sd0 — handled in storage

    auto* storage = AppContext::getFatFsStorage();
    bool success = storage->appendToFile(filepath, reinterpret_cast<const uint8_t*>(data), size);

    return success ? 0 : -1;
}

// Sends HTTP response to client
void MultipartParser::sendHttpResponse(int statusCode, const std::string& message) {
    std::string response = "HTTP/1.1 " + std::to_string(statusCode) + " " +
                           ((statusCode == 200) ? "OK" : "Error") +
                           "\r\nContent-Type: application/json\r\n\r\n{\"message\":\"" + message + "\"}";
    lwip_send(clientSocket, response.c_str(), response.size(), 0);
}

int MultipartParser::file_exists(const char* filename) {
    std::string filepath = std::string("/") + filename;

    auto* storage = AppContext::getFatFsStorage();
    return storage->exists(filepath) ? 1 : 0;
}
