/**
 * @file HttpResponse.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>
#include <unordered_map>

class Response {
    int sock;
    int status_code = 200;
    std::unordered_map<std::string, std::string> headers;
    bool headerSent = false;  // track if we’ve sent the initial header

    // Member variables for headers and other details
    std::string content_type = "text/html";  // Default content type
    std::string content_length;  // Will be set when we know the content size

    // Additional headers as needed
    

public:
    explicit Response(int sock);
    
    // Chainable method to set the HTTP status code.
    Response& status(int code);
    
    // Chainable method to set a header field.
    Response& set(const std::string &field, const std::string &value);

    Response& setAuthorization(const std::string &jwtToken);  // New method for JWT authorization   

    
    // Sends the response with the given body.
    void send(const std::string &body);

    // For streaming in chunks:
    void start(int code, size_t contentLength, const std::string &contentType = "application/octet-stream");
    void writeChunk(const char* data, size_t length);
    void finish();

    int getSocket() const;    // to retrieve the raw socket


    void setHeader(const std::string& key, const std::string& value);  // Optional helper method to set custom headers

    void sendHeaders();  // Helper to build & send initial headers once

    void sendUnauthorized();

private:
    std::string getStatusMessage(int code);
};

#endif // HTTPRESPONSE_HPP
