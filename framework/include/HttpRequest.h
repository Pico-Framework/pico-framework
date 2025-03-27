/**
 * @file HttpRequest.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
#pragma once

#include "utility.h"
#include <string>
#include <unordered_map>
#include <iostream>

class Router;  // Forward declaration

class Request {
public:

    // Constructor: pass in raw headers (as C-string), HTTP method, and request path.
    Request(const char* rawHeaders, const std::string& reqMethod, const std::string& reqPath); 

    std::string getHeader(const std::string &field) const {
        auto it = headers.find(toLower(field));
        return (it != headers.end()) ? it->second : "";  // Return empty string if not found
    }
    
    std::unordered_map<std::string, std::string> getHeaders() const {
        return headers;
    }
    
    void printHeaders() const {
        for (const auto& header : headers) {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }

    void setHeaderEnd(size_t end) {
        headerEnd = end;
    }

    size_t getHeaderEnd() {
        return headerEnd;
    }

    std::string getHost() const {
        return getHeader("host");
    }

    bool isFormUrlEncoded() const {
        auto it = headers.find("content-type");
        return it != headers.end() && it->second.find("application/x-www-form-urlencoded") != std::string::npos;
    }

    bool isJson() const {
        auto it = headers.find("content-type");
        return it != headers.end() && it->second.find("application/json") != std::string::npos;
    }

    // Method to get the body of the request
    const std::string& getBbody() const {
        return body;
    }

    // Method to set the body of the request (if needed elsewhere)
    void setBody(const std::string& aBody) {
        body = aBody;
    }

    std::string getBody(){
        return body;
    } 

    void setMethod(const std::string& aMethod) {
        method = aMethod;
    }

    void setPath(const std::string& aPath) {
        path = aPath;
    }

    std::string getMethod() const {
        return method;
    }
    
    std::string getPath() const {
        return path;
    }

    int getContentLength() const {
        std::string content_length_str = getHeader("content-length");
        int contentLength = 0;
        if (!content_length_str.empty()) {
            contentLength = std::stoi(content_length_str);
        }
        return contentLength;
    }

    bool isMultipart() const {
        auto contentType = getHeader("content-type");
        return contentType.find("multipart/form-data") != std::string::npos;
    }

    std::string getContentType() const {
        return getHeader("content-type");
    }

    std::string getBoundary() const {
        auto contentType = getContentType();
        auto boundaryPos = contentType.find("boundary=");
        if (boundaryPos != std::string::npos) {
            return contentType.substr(boundaryPos + 9);
        }
        return "";
    }

    void setClientIp(const std::string& ip) {
        clientIp = ip;
    }
    
    std::string getClientIp() const {
        return clientIp;
    }
    std::unordered_map<std::string, std::string> getCookies() const;
    std::string getCookie(const std::string& name) const;

    static Request receive(int clientSocket);
    static int receiveData(int clientSocket, char* buffer, int size);
    static bool getMethodAndPath(char* buffer, int clientSocket, char* method, char* path);
    std::unordered_map<std::string, std::string> getFormParams();
    std::unordered_map<std::string, std::string> getQueryParams();

    // multipart handling
    int handle_multipart(int new_sock, Request& req); 

private:
    void parseHeaders(const char* raw);
    std::string decodeUrl(const std::string& str);
    std::string clientIp; // Client IP address for logging or other purposes
    std::string method; // HTTP method (GET, POST, etc.)
    std::string url; // Original URL (before parsing)    
    std::string path; // Parsed path (after removing query string)
    std::string query; // Query string (if any)
    std::unordered_map<std::string, std::string> headers;
    std::string body;  // Internal storage for the body    
    size_t headerEnd = 0;


};

#endif // HTTPREQUEST_HPP
