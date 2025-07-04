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

#include "http/HttpFileserver.h"

#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"
TRACE_INIT(HttpFileserver)

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <lwip/sockets.h>
#include <unordered_map>

#include <FreeRTOS.h>
#include <task.h>

#include "framework/AppContext.h"
#include "utility/utility.h"
#include "http/url_utils.h"
#include "storage/StorageManager.h"
#include "framework/AppContext.h"
#include "framework_config.h"
#include "http/JsonResponse.h"

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
    StorageManager *storage = AppContext::get<StorageManager>();
    if (storage->mount())
    {
        TRACE("Storage mounted successfully\n");
        return true;
    }
    else
    {
        printf("Storage mount failed\n");
        return false;
    }
}

/// @copydoc FileHandler::listDirectory
bool FileHandler::listDirectory(const std::string &path, std::vector<FileInfo> &out)
{
    auto *storage = AppContext::get<StorageManager>();
    return storage->listDirectory(path, out);
}

/// @copydoc FileHandler::serveFile
bool FileHandler::serveFile(HttpResponse &res, const char *uri)
{
    std::string path = uri;

    storageManager = AppContext::get<StorageManager>();
    if (!storageManager->mount())
    {
        printf("Storage mount failed\n");
        JsonResponse::sendError(res, 500, "MOUNT_FAILED", "Storage mount failed");
        return false;
    }

    if (!storageManager->exists(path))
    {
        if (!storageManager->isMounted())
        {
            TRACE("Storage is not mounted — static file access failed\n");
            JsonResponse::sendError(res, 500, "NOT_MOUNTED", "Storage not mounted");
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
    TRACE("Serving file: %s, size: %zu bytes, MIME type: %s\n", path.c_str(), fileSize, mimeType.c_str());

    if (mimeType == "text/html" || mimeType == "application/javascript" || mimeType == "text/css")
    {
        // Gzip magic number: 0x1F 0x8B (in little-endian)
        const uint8_t gzip_magic_number[] = {0x1F, 0x8B};
        std::string magic_number;
        if (storageManager->readFileString(path, 0, 2, magic_number))
        {
            TRACE("Read magic number in hex: ");
            for (size_t i = 0; i < magic_number.size(); ++i)
            {
                TRACE("%02X ", static_cast<unsigned char>(magic_number[i]));
            }
            TRACE("\n");
            if (magic_number[0] == gzip_magic_number[0] && magic_number[1] == gzip_magic_number[1]) // GZIP magic number
            {
                TRACE("File is already gzipped: %s\n", path.c_str());
                TRACE("Setting Content-Encoding to gzip\n");
                res.set("Content-Encoding", "gzip");
            }
        }
    }

    res.start(200, fileSize, mimeType.c_str());

    storageManager->streamFile(path, [&](const uint8_t *data, size_t len)
                               {
        res.writeChunk(reinterpret_cast<const char*>(data), len);
        vTaskDelay(pdMS_TO_TICKS(STREAM_SEND_DELAY_MS)); }); // allow tcpip thread to get in

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
void HttpFileserver::handle_list_directory(HttpRequest &req, HttpResponse &res, const RouteMatch &match)
{
    std::string directory_path = req.getPath();
    int pos = directory_path.find("/api/v1/ls");
    if (pos != std::string::npos)
    {
        directory_path = directory_path.substr(pos + strlen("/api/v1/ls"));
    }

    if (directory_path.empty())
    {
        directory_path = "/"; // Default to root directory if no path is specified
    }

    std::vector<FileInfo> entries;
    if (!fileHandler.listDirectory(directory_path, entries))
    {
        res.sendError(404, "not_found", "Directory not found or inaccessible");
        return;
    }

    nlohmann::json fileArray = nlohmann::json::array();
    for (const auto &entry : entries)
    {
        fileArray.push_back({{"name", entry.name},
                             {"size", entry.size},
                             {"type", entry.isDirectory ? "directory" : "file"}});
    }

    nlohmann::json result = {
        {"path", directory_path},
        {"files", fileArray}};

    res.sendSuccess(result, "Directory listed successfully.");
}

// Helper function to check if a string ends with a given suffix
bool ends_with(const std::string &str, const std::string &suffix)
{
    if (str.length() < suffix.length())
        return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

// Fallback route for serving any static file detected in router
/// @copydoc HttpFileserver::handle_static_request
void HttpFileserver::handle_static_request(HttpRequest &req, HttpResponse &res, const RouteMatch &match)
{
    const std::string &uri = req.getPath();
    printf("Serving static request for URI: %s\n", uri.c_str());

    std::string filePath = urlDecode(uri);
    printf("Decoded URI: %s\n", filePath.c_str());

    if (uri.empty() || uri == "/")
    {
        filePath = "/index.html";
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
        {".gz", "application/x-gzip-compressed"},
        {".tar", "application/x-tar"},
        {".mp4", "video/mp4"},
        {".webm", "video/webm"},
        {".ogg", "audio/ogg"},
        {".flac", "audio/flac"},
        {".aac", "audio/aac"},
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
