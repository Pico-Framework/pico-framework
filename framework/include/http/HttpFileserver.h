/**
 * @file HttpFileserver.h
 * @author Ian Archbell
 * @brief HTTP file server and file handling helpers for static content.
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 *
 */

#ifndef HTTP_FILE_SERVER_H
#define HTTP_FILE_SERVER_H
#pragma once

#include <string>
#include <vector>

#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "storage/StorageManager.h"

/**
 * @brief Helper class for accessing the file system and serving file content.
 */
class FileHandler
{
public:
    /**
     * @brief Construct a new FileHandler object.
     */
    FileHandler();

    /**
     * @brief Initialize and mount the storage (e.g., SD card).
     * @return true if mounted successfully.
     */
    bool init();

    /**
     * @brief Return a list of a given directory.
     * @param path Directory path to list.
     */
    bool listDirectory(const std::string& path, std::vector<FileInfo>& out);

    /**
     * @brief Serve a file to the client via the HttpResponse object.
     * @param res HTTP response object.
     * @param uri URI or path to the file.
     * @return true if file was served successfully.
     */
    bool serveFile(HttpResponse &res, const char *uri);

    /**
     * @brief Flag indicating whether storage is mounted.
     */
    bool mounted = false;

private:
    StorageManager *storageManager;
};

/**
 * @brief HTTP-level controller for serving static files and directory listings.
 */
class HttpFileserver
{
public:
    /**
     * @brief Construct a new HttpFileserver object.
     */
    HttpFileserver();

    /**
     * @brief Handle requests for static file content.
     * @param req The HTTP request object.
     * @param res The HTTP response object.
     * @param params Route parameters (unused).
     */
    void handle_static_request(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params);

    /**
     * @brief Handle requests to list directory contents.
     * @param req The HTTP request object.
     * @param res The HTTP response object.
     * @param params Route parameters (unused).
     */
    void handle_list_directory(HttpRequest &req, HttpResponse &res, const std::vector<std::string> &params);

    /**
     * @brief Get the MIME type based on the file extension.
     * @param filePath Full file path.
     * @return MIME type string.
     */
    std::string getMimeType(const std::string &filePath);

private:
    FileHandler fileHandler;
};

#endif // HTTP_FILE_SERVER_H
