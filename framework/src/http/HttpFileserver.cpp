/**
 * @file HttpFileserver.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "HttpFileserver.h"
#include <ff_utils.h>
#include <ff_stdio.h>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <lwip/sockets.h>
#include "utility.h"

#define TRACE_ON

FileHandler::FileHandler() {

}

bool FileHandler::init() {
    if (mount("sd0")) {
        TRACE("SD Card mounted successfully\n");
        return true;
    } else {
        printf("SD Card mount failed\n");
        return false;
    }
}

void FileHandler::listDirectory(const char *path) {
    printf("Listing directory: %s\n", path);
    char pcWriteBuffer[128] = {0};
    FF_FindData_t xFindStruct;
    memset(&xFindStruct, 0x00, sizeof(FF_FindData_t));

    if (!path) ff_getcwd(pcWriteBuffer, sizeof(pcWriteBuffer));
    printf("Directory Listing: %s\n", path ? path : pcWriteBuffer);

    int iReturned = ff_findfirst(path ? path : "", &xFindStruct);
    if (FF_ERR_NONE != iReturned) {
        printf("ff_findfirst error: %s)\n", iReturned);
        return;
    }
    do {
        const char *pcAttrib = (xFindStruct.ucAttributes & FF_FAT_ATTR_DIR) ? "directory" :
                                (xFindStruct.ucAttributes & FF_FAT_ATTR_READONLY) ? "read only file" : "writable file";
        printf("%s\t[%s]\t[size=%lu]\n", xFindStruct.pcFileName, pcAttrib, xFindStruct.ulFileSize);
    } while (FF_ERR_NONE == ff_findnext(&xFindStruct));
}

std::string getMimeType(const std::string& filePath) {
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
        {".csv", "text/csv"}
    };

    size_t extPos = filePath.find_last_of(".");
    if (extPos != std::string::npos) {
        std::string ext = filePath.substr(extPos); // Extract extension
        TRACE("Extracted extension: %s\n", ext.c_str());  // Debug to check the extension

        auto it = mimeTypes.find(ext);
        if (it != mimeTypes.end()) {
            return it->second;  // Return corresponding MIME type
        }
    }

    return "application/octet-stream";  // Default MIME type
}


bool FileHandler::serveFile(Response &res, const char *uri) {
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "/sd0%s", uri);
    FF_FILE* file = ff_fopen(filepath, "r");
    if (!file) {
        printf("File not found: %s\n", filepath);
        // Use new approach: 404
        res.status(404).send("File Not Found: " + std::string(uri));
        return false;
    }

    // figure out file size
    ff_fseek(file, 0, SEEK_END);
    long file_size = ff_ftell(file);
    ff_fseek(file, 0, SEEK_SET);

    if (file_size < 0) {
        ff_fclose(file);
        res.status(404).send("Error reading file size for: " + std::string(uri));
        return false;
    }

    // Determine the correct Content-Type
    std::string mimeType = getMimeType(filepath);

    printf("Serving file: %s, size: %ld bytes, MIME type: %s\n", filepath, file_size, mimeType.c_str());

    // 1) start() the response with known size
    TRACE("Starting response with size: %ld\n", file_size);
    std::string file_path = "/sd0" + std::string(uri);
    res.start(200, static_cast<size_t>(file_size), mimeType.c_str());

    // 2) read in schunks and call writeChunk
    char buffer[512];
    uint32_t bytes_read;
    TRACE("Reading file in chunks...\n");
    do {
        bytes_read = ff_fread(buffer, 1, sizeof(buffer), file);
        if (bytes_read > 0) {
            res.writeChunk(buffer, bytes_read);
            vTaskDelay(20);  // Small delay to allow tcpip thread to send data
        }
    } while (bytes_read > 0);

    ff_fclose(file);

    // 3) finish() the response (no real effect with content-length, but neat)
    res.finish();
    TRACE("Called res finish\n");

    return true;
}

HttpFileServer::HttpFileServer() {
    fileHandler.init();
}

void HttpFileServer::handle_list_directory(Request &req, Response &res, const std::vector<std::string> &params){
    std::string directory_path = req.getPath();
    printf("Handling list directory request for URI: %s\n", directory_path.c_str());

    // Extract directory path from URI
    int pos = directory_path.find("/api/v1/ls");
    if (pos != std::string::npos) {
        directory_path = directory_path.substr(pos + strlen("/api/v1/ls"));
    }

    fileHandler.listDirectory(directory_path.c_str());

    const char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nDirectory listed successfully.";
    int client_socket = res.getSocket();
    lwip_send(client_socket, response, strlen(response), 0);
}

// Helper function to check if a string ends with a given suffix
bool ends_with(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

void HttpFileServer::handle_static_request(Request &req, Response &res, const std::vector<std::string> &params)
{
    const std::string &uri = req.getPath();
    printf("Serving static request for URI: %s\n", uri.c_str());

    std::string filePath;

    // If the URI is empty or "/", serve index.html
    if (uri.empty() || uri == "/") {
        filePath = "/index.html";
    } else {
        filePath = uri;
    }
    
    // Serve the file d now that headers are set
    fileHandler.serveFile(res, filePath.c_str());
}

