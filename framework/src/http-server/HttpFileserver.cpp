/**
 * @file HttpFileserver.cpp
 * @author Ian Archbell
 * @brief Implementation of file serving and directory listing over HTTP.
 *
 * Part of the PicoFramework HTTP server.
 * This module handles serving files from the SD card and listing directory contents.
 * It uses the FatFsStorageManager to interact with the filesystem.
 *
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"
TRACE_INIT(HttpFileserver)

#include "HttpFileserver.h"
#include "AppContext.h"
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <lwip/sockets.h>
#include <unordered_map>

#include "utility.h"
#include "url_utils.h"
#include "FatFsStorageManager.h"
#include "AppContext.h"
#include "framework_config.h"
#include "JsonResponse.h"

#define TRACE_ON

// ----------------------------------------------------------------------------
// FileHandler Implementation
// ----------------------------------------------------------------------------

/// @copydoc FileHandler::FileHandler
FileHandler::FileHandler()
{
}

/// @copydoc FileHandler::init
bool FileHandler::init()
{
    FatFsStorageManager *storage = AppContext::getInstance().getService<FatFsStorageManager>();
    if (storage->mount())
    {
        TRACE("SD Card mounted successfully\n");
        return true;
    }
    else
    {
        printf("SD Card mount failed\n");
        return false;
    }
}

/// @copydoc FileHandler::listDirectory
void FileHandler::listDirectory(const char *path)
{
    FatFsStorageManager *storage = AppContext::getInstance().getService<FatFsStorageManager>();
    if (!storage)
    {
        printf("No storage manager available\n");
        return;
    }

    std::string dir = path ? path : "/";
    std::vector<FileInfo> files;

    if (!storage->listDirectory(dir, files))
    {
        printf("Failed to list directory: %s\n", dir.c_str());
        return;
    }

    printf("Directory Listing: %s\n", dir.c_str());
    for (const auto &f : files)
    {
        const char *attr = f.isDirectory ? "directory" : f.isReadOnly ? "read-only file"
                                                                      : "writable file";

        printf("%s\t[%s]\t[size=%zu]\n", f.name.c_str(), attr, f.size);
    }
}

/// @copydoc FileHandler::serveFile
bool FileHandler::serveFile(HttpResponse &res, const char *uri)
{
    std::string path = uri;

    storageManager = AppContext::getInstance().getService<FatFsStorageManager>();

    if (!storageManager->exists(path))
    {
        if (!storageManager->isMounted())
        {
            TRACE("SD card is not mounted — static file access failed\n");
            JsonResponse::sendError(res, 500, "SD_CARD_NOT_MOUNTED", "SD card not mounted");
            return false;
        }
        else
        {
            printf("File not found: %s\n", path.c_str());
            JsonResponse::sendError(res, 404, "NOT_FOUND", "File Not Found: " + std::string(uri));
            return false;
        }
    }

    size_t fileSize = storageManager->getFileSize(path);
    if (fileSize == 0)
    {
        JsonResponse::sendError(res, 500, "FILESIZE_ERROR", "Error getting file size for: " + std::string(uri));
        return false;
    }

    std::string mimeType = getMimeType(path);
    printf("Serving file: %s, size: %zu bytes, MIME type: %s\n", path.c_str(), fileSize, mimeType.c_str());

    res.start(200, fileSize, mimeType.c_str());

    storageManager->streamFile(path, [&](const uint8_t *data, size_t len)
                               {
         res.writeChunk(reinterpret_cast<const char*>(data), len);
         vTaskDelay(pdMS_TO_TICKS(STREAM_SEND_DELAY_MS)); });

    res.finish();
    return true;
}

// ----------------------------------------------------------------------------
// HttpFileserver Implementation
// ----------------------------------------------------------------------------

/// @copydoc HttpFileserver::HttpFileserver
HttpFileserver::HttpFileserver()
{
    // fileHandler.init();  // switched to lazy init to avoid startup issues
}

/// @copydoc HttpFileserver::handle_list_directory
void HttpFileserver::handle_list_directory(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
{
    std::string directory_path = req.getPath();
    printf("Handling list directory request for URI: %s\n", directory_path.c_str());

    int pos = directory_path.find("/api/v1/ls");
    if (pos != std::string::npos)
    {
        directory_path = directory_path.substr(pos + strlen("/api/v1/ls"));
    }

    fileHandler.listDirectory(directory_path.c_str());

    const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nDirectory listed successfully.";
    int client_socket = res.getSocket();
    lwip_send(client_socket, response, strlen(response), 0);
}

// Helper function to check if a string ends with a given suffix
bool ends_with(const std::string &str, const std::string &suffix)
{
    if (str.length() < suffix.length())
        return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

/// @copydoc HttpFileserver::handle_static_request
void HttpFileserver::handle_static_request(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params)
{
    const std::string &uri = req.getPath();
    printf("Serving static request for URI: %s\n", uri.c_str());

    std::string filePath;

    if (uri.empty() || uri == "/")
    {
        filePath = "/index.html";
    }
    else
    {
        filePath = uri;
    }

    fileHandler.serveFile(res, filePath.c_str());
}

/// @copydoc HttpFileserver::getMimeType
std::string HttpFileserver::getMimeType(const std::string &filePath)
{
    TRACE("Getting MIME type for file: %s\n", filePath.c_str());

    static const std::unordered_map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".txt", "text/plain"},
        {".xml", "application/xml"},
        {".pdf", "application/pdf"},
        {".zip", "application/zip"},
        {".mp4", "video/mp4"},
        {".mp3", "audio/mpeg"},
        {".wav", "audio/wav"},
        {".csv", "text/csv"}};

    size_t extPos = filePath.find_last_of(".");
    if (extPos != std::string::npos)
    {
        std::string ext = filePath.substr(extPos);
        TRACE("Extracted extension: %s\n", ext.c_str());

        auto it = mimeTypes.find(ext);
        if (it != mimeTypes.end())
        {
            return it->second;
        }
    }

    return "application/octet-stream";
}
