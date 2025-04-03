/**
 * @file MultipartParser.cpp
 * @author Ian Archbell
 * @brief Implementation of multipart/form-data parsing and file upload handling.
 * 
 * Part of the PicoFramework HTTP server.
 * This module processes multipart/form-data uploads, detects boundaries,
 * parses headers, extracts filenames, and writes file data to storage.
 * It also handles sending appropriate HTTP responses based on the success
 * or failure of the upload process.
 * 
 * Extracts the boundary from the Content-Type header
 * Manages multipart state using a basic state machine (SEARCHING_FOR_BOUNDARY, FOUND_BOUNDARY, etc.)
 * Used a streaming lwip_recv() loop
 * Bufferes data and handles chunk boundaries
 * Writes file data incrementally (appends to file to cope with large file sizes)
 * Rejects uploads with duplicate filenames
 * Only supports one file per request (for simplicity)
 * 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
 #include "DebugTrace.h" 
 TRACE_INIT(MultipartParser)
 
 #include "MultipartParser.h"
 #include <lwip/sockets.h>
 #include <iostream>
 #include <cstring>
 #include <sstream>
 #include <fstream>
 #include <algorithm>
 #include <cctype>
 #include <filesystem>
 #include "AppContext.h" 
 
State MultipartParser::currentState = SEARCHING_FOR_BOUNDARY; // Initialize state

 /// @copydoc MultipartParser::MultipartParser
 MultipartParser::MultipartParser(int clientSocket, const Request& request)
     : clientSocket(clientSocket), request(request)
 {
     std::string contentType = request.getHeader("Content-Type");
     std::size_t pos = contentType.find("boundary=");
     if (pos != std::string::npos)
     {
         boundary = contentType.substr(pos + 9);
     }
 }
 
 /// @copydoc MultipartParser::handleMultipart
 bool MultipartParser::handleMultipart()
{
    char buf[1460];
    int len;

    currentState = SEARCHING_FOR_BOUNDARY;

    // Handle any body data included with the initial request
    const std::string& initialBody = request.getBody();
    if (!initialBody.empty())
    {
        std::string chunk = initialBody;
        if (!handleChunk(chunk))
            return false;
    }

    // Stream remaining data from socket
    while ((len = lwip_recv(clientSocket, buf, sizeof(buf) - 1, 0)) > 0)
    {
        buf[len] = '\0';
        std::string chunk(buf, len);

        if (!handleChunk(chunk))
            return false;

        if (currentState == COMPLETE)
            break;

        vTaskDelay(pdMS_TO_TICKS(10)); // yield to allow other tasks to run
    }

    if (currentState != COMPLETE)
    {
        sendHttpResponse(400, "Upload incomplete or failed");
        return false;
    }

    // Only send 200 when we're truly done
    sendHttpResponse(200, "File uploaded successfully");
    return true;
}
 
 /// @copydoc MultipartParser::handleChunk
 bool MultipartParser::handleChunk(std::string& chunkData)
 {
     printf("Current state on handling chunk: %d\n", currentState);
     printf("Handling chunk data, size: %zu bytes\n", chunkData.size());
 
     const std::string boundaryPrefix = "--" + boundary;
     const std::string finalBoundary = boundaryPrefix + "--";
 
     while (!chunkData.empty() && currentState != COMPLETE)
     {
         switch (currentState)
         {
             case SEARCHING_FOR_BOUNDARY:
             {
                 size_t boundaryPos = chunkData.find(boundaryPrefix);
                 if (boundaryPos != std::string::npos)
                 {
                     size_t skip = boundaryPos + boundaryPrefix.length();
                     if (chunkData.substr(skip, 2) == "\r\n")
                         skip += 2;
                     chunkData = chunkData.substr(skip);
                     currentState = FOUND_BOUNDARY;
                     printf("Found initial boundary, buffer size now: %zu bytes\n", chunkData.size());
                     continue;
                 }
                 return true; // Wait for more data
             }
 
             case FOUND_BOUNDARY:
            {
                size_t headersEnd = chunkData.find("\r\n\r\n");
                if (headersEnd == std::string::npos)
                    return true; // wait for more data

                std::string headers = chunkData.substr(0, headersEnd);
                std::istringstream stream(headers);
                std::string line;
                bool gotFilename = false;

                while (std::getline(stream, line))
                {
                    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
                    if (line.find("Content-Disposition:") != std::string::npos)
                    {
                        gotFilename = extractFilename(line);
                    }
                }

                if (!gotFilename)
                {
                    sendHttpResponse(400, "Invalid upload: no filename");
                    currentState = COMPLETE;
                    return false;
                }

                // If file exists already, send error
                 
                if (file_exists(filename.c_str()))
                {
                    sendHttpResponse(409, "File already exists");
                    currentState = COMPLETE;
                    return false;
                }

                chunkData = chunkData.substr(headersEnd + 4); // skip headers
                currentState = FOUND_DATA_START;
                continue;
            }

 
             case FOUND_DATA_START:
             {
                 size_t finalPos = chunkData.find(finalBoundary);
                 size_t nextBoundaryPos = chunkData.find(boundaryPrefix);
 
                 size_t boundaryPos = std::string::npos;
                 bool isFinal = false;
 
                 if (finalPos != std::string::npos &&
                     (nextBoundaryPos == std::string::npos || finalPos < nextBoundaryPos))
                 {
                     boundaryPos = finalPos;
                     isFinal = true;
                 }
                 else if (nextBoundaryPos != std::string::npos)
                 {
                     boundaryPos = nextBoundaryPos;
                 }
 
                 if (boundaryPos == std::string::npos)
                 {
                     // No boundary, stream all data
                     if (processFileData(chunkData))
                     {
                         chunkData.clear();
                         return true;
                     }
                     return false; // Error processing data;
                 }
 
                 // Process data up to boundary
                 size_t dataEnd = boundaryPos;
                 if (dataEnd >= 2 && chunkData.substr(dataEnd - 2, 2) == "\r\n")
                     dataEnd -= 2;
 
                 std::string fileData = chunkData.substr(0, dataEnd);
                 processFileData(fileData);
 
                 // Advance past boundary
                 size_t skip = boundaryPos + boundaryPrefix.length();
                 if (isFinal)
                     skip += 2; // "--"
                 if (chunkData.substr(skip, 2) == "\r\n")
                     skip += 2;
 
                 chunkData = chunkData.substr(skip);
                 currentState = isFinal ? COMPLETE : FOUND_BOUNDARY;
                 continue;
             }
 
             default:
                 return false;
         }
     }
 
     return true;
 }
 
 
 /// @copydoc MultipartParser::extractFilename
 bool MultipartParser::extractFilename(const std::string& contentDisposition)
 {
     std::size_t start = contentDisposition.find("filename=\"");
     if (start == std::string::npos) return false;
 
     start += 10;
     std::size_t end = contentDisposition.find("\"", start);
     if (end == std::string::npos) return false;
 
     filename = contentDisposition.substr(start, end - start);
 
     if (file_exists(filename.c_str()))
     {
         sendHttpResponse(400, "File already exists");
         return false;
     }
 
     return true;
 }
 
 /// @copydoc MultipartParser::processFileData
 bool MultipartParser::processFileData(const std::string& fileData)
 {
    printf("Processing file data, size: %zu bytes\n", fileData.size()); 
    if(!appendFileToSD(filename.c_str(), fileData.c_str(), fileData.size())){ 
        sendHttpResponse(500, "Failed to write file data");
        return false;
    }
    return true;
 }
 
 /// @copydoc MultipartParser::appendFileToSD
 int MultipartParser::appendFileToSD(const char* filename, const char* data, size_t size)
 {
    printf("Appending %zu bytes to file: %s\n", size, filename);
 
     // Append data to the file using FatFsStorageManager
    FatFsStorageManager* storage = AppContext::getFatFsStorage();
    return storage->appendToFile(filename, (uint8_t*)data, size); // Use the FatFsStorageManager to handle file appending
 }
 
 /// @copydoc MultipartParser::file_exists
 int MultipartParser::file_exists(const char* filename)
 {
    FatFsStorageManager* storage = AppContext::getFatFsStorage(); 
    return storage->exists(filename);
 }
 
 /// @copydoc MultipartParser::findDataStart
 int MultipartParser::findDataStart(std::string& currentData)
 {
     std::size_t pos = currentData.find("\r\n\r\n");
     return (pos != std::string::npos) ? static_cast<int>(pos + 4) : -1;
 }
 
 /// @copydoc MultipartParser::handleFinalBoundary
 bool MultipartParser::handleFinalBoundary(std::string& fileData)
 {
     std::size_t finalBoundaryPos = fileData.find(boundary + "--");
     if (finalBoundaryPos != std::string::npos)
     {
         fileData = fileData.substr(0, finalBoundaryPos);
         return true;
     }
     return false;
 }
 
 /// @copydoc MultipartParser::sendHttpResponse
 void MultipartParser::sendHttpResponse(int statusCode, const std::string& message)
 {
     std::ostringstream oss;
     oss << "HTTP/1.1 " << statusCode << " OK\r\n"
         << "Content-Type: text/plain\r\n\r\n"
         << message;
 
     lwip_send(clientSocket, oss.str().c_str(), oss.str().length(), 0);
 }
 