/**
 * @file HttpFileserver.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef HTTP_FILE_SERVER_H
#define HTTP_FILE_SERVER_H
#pragma once

#include <string>
#include <vector>
#include <ff_utils.h>
#include <ff_stdio.h>

#include "HttpRequest.h"
#include "HttpResponse.h"

class FileHandler {
public:
    FileHandler();
    bool init();
    void listDirectory(const char *path);
    bool serveFile(Response &res, const char* uri);

};

class HttpFileServer {
private:
    FileHandler fileHandler;

public:
    HttpFileServer();
    void handle_static_request(Request &req, Response &res, const std::vector<std::string> &params);
    void handle_list_directory(Request &req, Response &res, const std::vector<std::string> &params);  
    std::string getMimeType(const std::string& filePath); 
};

#endif // HTTP_FILE_SERVER_H
