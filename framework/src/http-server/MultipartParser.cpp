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
 * Used a streaming recv() loop
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

#include "http/MultipartParser.h"
#include <lwip/sockets.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>
//#include <filesystem>
#include <FreeRTOS.h>
#include <task.h>

#include "framework/AppContext.h"
#include "storage/StorageManager.h"

State MultipartParser::currentState = SEARCHING_FOR_BOUNDARY; // Initialize state

/// @copydoc MultipartParser::MultipartParser
MultipartParser::MultipartParser()

{

}

void MultipartParser::setBoundaryFromContentType(const std::string& contentType){
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos != std::string::npos) {
        boundary = contentType.substr(boundaryPos + 9); // 9 is the length of "boundary="
        // Remove any leading/trailing whitespace
        boundary.erase(0, boundary.find_first_not_of(" \t\r\n"));
        boundary.erase(boundary.find_last_not_of(" \t\r\n") + 1);
        TRACE("Boundary set to: '%s'\n", boundary.c_str());
    } else {
        boundary.clear();
        TRACE("No boundary found in Content-Type header\n");
    }
}

bool MultipartParser::handleMultipart(HttpRequest& req, HttpResponse& res)
{
    tcp = req.getTcp();  // extract here — don't store in ctor

    std::string contentType = req.getHeader("Content-Type");
    setBoundaryFromContentType(contentType);

    if (boundary.empty()) {
        res.status(400).send("Missing boundary");
        return false;
    }

    char buf[1460];
    int len;

    currentState = SEARCHING_FOR_BOUNDARY;

    // Handle any body data included with the initial request
    const std::string &initialBody = req.getBody();
    if (!initialBody.empty())
    {
        std::string chunk = initialBody;
        if (!handleChunk(chunk))
        {
            TRACE("Error handling chunk\n");
            return false;
        }
    }

    // Stream remaining data from socket
    while ((len = tcp->recv(buf, sizeof(buf) - 1)) > 0)
    {
        buf[len] = '\0';
        std::string chunk(buf, len);

        if (!handleChunk(chunk))
        {
            sendHttpResponse(400, "Upload incomplete or failed");
            TRACE("Error handling chunk\n");
            return false;
        }

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
    TRACE("Multipart: Successfully received file '%s'\n", filename.c_str());
    sendHttpResponse(200, "File uploaded successfully");
    return true;
}

/// @copydoc MultipartParser::handleChunk
bool MultipartParser::handleChunk(std::string &chunkData)
{
    TRACE("Handling chunk, size: %zu bytes\n", chunkData.size());
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
                TRACE("Found initial boundary, buffer size now: %zu bytes\n", chunkData.size());
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
                    if (gotFilename)
                    {
                        TRACE("Filename extracted: %s\n", filename.c_str());
                    }
                }
            }

            if (!gotFilename)
            {
                sendHttpResponse(400, "Invalid upload: no filename or filename exists already");
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
            isFinal = (chunkData.substr(skip, 2) == "--");
            if (isFinal)
                skip += 2;

            if (chunkData.substr(skip, 2) == "\r\n")
                skip += 2;

            chunkData = chunkData.substr(skip);

            // Any non-final boundary = unsupported multiple parts
            if (!isFinal)
            {
                TRACE("ERROR: Multipart contains multiple file parts — unsupported\n");
                sendHttpResponse(400, "Multiple file upload not supported");
                return false;
            }

            TRACE("Final boundary reached — exiting multipart parser\n");
            currentState = COMPLETE; // ✅ <== this was missing
            break;

            TRACE("Final boundary reached — exiting multipart parser\n");
            break;
        }

        default:
            return false;
        }
    }

    return true;
}

/// @copydoc MultipartParser::extractFilename
bool MultipartParser::extractFilename(const std::string &contentDisposition)
{
    std::size_t start = contentDisposition.find("filename=\"");
    if (start == std::string::npos)
        return false;

    start += 10;
    std::size_t end = contentDisposition.find("\"", start);
    if (end == std::string::npos)
        return false;
    // Ensure upload directory exists
    auto storage = AppContext::getInstance().getService<StorageManager>();
    if(!storage) {
        printf("[MultipartParser] StorageManager service not available\n");
        sendHttpResponse(500, "StorageManager service not available");
        return false;
    }
    storage->mount(); // ensure mounted
    std::string uploads(MULTIPART_UPLOAD_PATH); 
    TRACE("Checking if upload directory exists: %s\n", uploads.c_str());
    if (!storage->exists(uploads)) {
        TRACE("Upload directory does not exist, creating: %s\n", uploads.c_str());
        if(!(storage->createDirectory(uploads))) {
            TRACE("Failed to create upload directory: %s\n", uploads.c_str());
            sendHttpResponse(500, "Failed to create upload directory");
            return false;
        }
    }
    filename = std::string(MULTIPART_UPLOAD_PATH) + "/" + contentDisposition.substr(start, end - start);
    TRACE("checking if filename: %s exists\n", filename.c_str());
    if (file_exists(filename))
    {
        printf("[MultipartParser] File already exists: %s\n", filename.c_str());
        return false;
    }
    return true;
}

/// @copydoc MultipartParser::processFileData
bool MultipartParser::processFileData(const std::string &fileData)
{
    TRACE("Processing file data, size: %zu bytes\n", fileData.size());
    auto *storage = AppContext::getInstance().getService<StorageManager>();
    if (!storage->appendToFile(filename, (uint8_t *)fileData.c_str(), fileData.size()))
    {
        if (!storage->isMounted())
        {
            TRACE("SD card not mounted — cannot handle multipart upload\n");
            sendHttpResponse(500, "SD card not mounted");
        }
        else
        {
            TRACE("Failed to append file during multipart upload\n");
            sendHttpResponse(500, "Failed to write file data");
        }
        return false;
    }
    return true;
}

/// @copydoc MultipartParser::file_exists
int MultipartParser::file_exists(const std::string& filename)
{
    StorageManager *storage = AppContext::getInstance().getService<StorageManager>();
    printf("[MultipartParser] Checking if file exists: %s\n", filename.c_str());
    return storage->exists(filename);
}

/// @copydoc MultipartParser::findDataStart
int MultipartParser::findDataStart(std::string &currentData)
{
    std::size_t pos = currentData.find("\r\n\r\n");
    return (pos != std::string::npos) ? static_cast<int>(pos + 4) : -1;
}

/// @copydoc MultipartParser::handleFinalBoundary
bool MultipartParser::handleFinalBoundary(std::string &fileData)
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
void MultipartParser::sendHttpResponse(int statusCode, const std::string &message)
{
    std::string code = std::to_string(statusCode);
    std::string success = (statusCode >= 200 && statusCode < 300) ? "true" : "false";
    std::string body = R"({"success":)" + success + R"(,"error":{"code":")" + code +
                       R"(","message":")" + message + R"("}})";

    std::ostringstream oss;
    oss << "HTTP/1.1 " << statusCode << " Bad Request\r\n"
        << "Content-Type: application/json\r\n"
        << "Content-Length: " << body.length() << "\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;

    std::string response = oss.str();
    if (!tcp || !tcp->isConnected()) {
        panic("Attempted to send on invalid socket");
    }
    runTimeStats();
    tcp->send(response.c_str(), response.length());
    vTaskDelay(pdMS_TO_TICKS(50)); // yield to allow send to complete
}
