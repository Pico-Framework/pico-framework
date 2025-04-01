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
 
 /// @copydoc MultipartParser::MultipartParser
 MultipartParser::MultipartParser(int clientSocket, const Request& request)
     : clientSocket(clientSocket), request(request)
 {
     std::string contentType = request.getHeader("Content-Type");
     std::size_t pos = contentType.find("boundary=");
     if (pos != std::string::npos)
     {
         boundary = "--" + contentType.substr(pos + 9);
     }
 }
 
 /// @copydoc MultipartParser::handleMultipart
 bool MultipartParser::handleMultipart()
 {
     char buf[1024];
     int len;
 
     while ((len = lwip_recv(clientSocket, buf, sizeof(buf) - 1, 0)) > 0)
     {
         buf[len] = '\0';
         std::string chunk(buf, len);
 
         if (!leftoverData.empty())
         {
             chunk = leftoverData + chunk;
             leftoverData.clear();
         }
 
         if (!handleChunk(chunk))
         {
             return false;
         }
     }
 
     return true;
 }
 
 /// @copydoc MultipartParser::handleChunk
 bool MultipartParser::handleChunk(std::string& chunkData)
 {
     buffer += chunkData;
 
     if (currentState == SEARCHING_FOR_BOUNDARY)
     {
         size_t boundaryPos = buffer.find(boundary);
         if (boundaryPos != std::string::npos)
         {
             buffer = buffer.substr(boundaryPos + boundary.length() + 2); // skip CRLF
             currentState = FOUND_BOUNDARY;
         }
         else
         {
             return true;
         }
     }
 
     if (currentState == FOUND_BOUNDARY)
     {
         std::istringstream stream(buffer);
         std::string line;
         bool headersComplete = false;
 
         while (std::getline(stream, line))
         {
             line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
             if (line.empty())
             {
                 headersComplete = true;
                 break;
             }
             if (line.find("Content-Disposition:") != std::string::npos)
             {
                 if (!extractFilename(line))
                 {
                     sendHttpResponse(400, "Invalid file upload: missing filename");
                     return false;
                 }
             }
         }
 
         if (!headersComplete)
         {
             return true; // Wait for more data
         }
 
         std::string remaining((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
         buffer = remaining;
         currentState = FOUND_DATA_START;
     }
 
     if (currentState == FOUND_DATA_START)
     {
         size_t boundaryPos = buffer.find(boundary);
         if (boundaryPos != std::string::npos)
         {
             std::string fileData = buffer.substr(0, boundaryPos - 2); // -2 to remove CRLF
             processFileData(fileData);
             buffer = buffer.substr(boundaryPos + boundary.length() + 2);
             currentState = FOUND_BOUNDARY;
         }
         else
         {
             leftoverData = buffer;
             processFileData(buffer);
             buffer.clear();
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
     return appendFileToSD(filename.c_str(), fileData.c_str(), fileData.size()) == 0;
 }
 
 /// @copydoc MultipartParser::appendFileToSD
 int MultipartParser::appendFileToSD(const char* filename, const char* data, size_t size)
 {
     std::ofstream file(filename, std::ios::binary | std::ios::app);
     if (!file) return 1;
     file.write(data, size);
     return 0;
 }
 
 /// @copydoc MultipartParser::file_exists
 int MultipartParser::file_exists(const char* filename)
 {
     return std::filesystem::exists(filename) ? 1 : 0;
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
 