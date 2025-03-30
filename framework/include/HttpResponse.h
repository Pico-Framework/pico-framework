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
#include <map>
#include <vector>

class Response {
    int sock;
    int status_code = 200;
    std::unordered_map<std::string, std::string> headers;
    bool headerSent = false;  // track if we’ve sent the initial header

    // Member variables for headers and other details
    std::string content_length;  // Will be set when we know the content size
    
public:
    explicit Response(int sock);
    //explicit Response(TcpConnectionSocket& socket);
    
    // Chainable method to set the HTTP status code.
    Response& status(int code);
    Response& setStatus(int code); // alias

    Response& setContentType(const std::string &content_type);  // Set the content type for the response
    std::string getContentType() const {
        auto it = headers.find("Content-Type");
        if (it != headers.end()) {
            return it->second;
        }
        return "text/html";  // Default content type
    }   
    
    // Chainable method to set a header field.
    Response& set(const std::string &field, const std::string &value);

    Response& setAuthorization(const std::string &jwtToken);  // New method for JWT authorization   

    Response& setCookie(const std::string& name, const std::string& value, const std::string& options);
    Response& clearCookie(const std::string& name, const std::string& options);
   
    // Sends the response with the given body.
    void send(const std::string &body);

    // For streaming in chunks:
    void start(int code, size_t contentLength, const std::string &contentType = "application/octet-stream");
    void writeChunk(const char* data, size_t length);
    void finish();

    int getSocket() const;    // to retrieve the raw socket


    Response& setHeader(const std::string& key, const std::string& value);  // Optional helper method to set custom headers

    void sendHeaders();  // Helper to build & send initial headers once

    Response& redirect(const std::string& url, int statusCode) {
        status(statusCode);
        set("Location", url);
        send("");  // No body
        return *this;
    }

    void sendNotFound() {
        status(404)
            .set("Content-Type", "application/json")
            .send("{\"error\": \"Not Found\"}");
    }
    
    void endServerError(const std::string& msg) {
        status(500)
            .set("Content-Type", "application/json")
            .send("{\"error\": \"" + msg + "\"}");
    }

    bool isHeaderSent() const {
        return headerSent;
    }
    
    Response& json(const std::string& jsonString) {
        set("Content-Type", "application/json");
        send(jsonString);
        return *this;
    }
    
    Response& text(const std::string& textString) {
        set("Content-Type", "text/plain");
        send(textString);
        return *this;
    }

    void sendUnauthorized();

    std::string renderTemplate(const std::string& tpl, const std::map<std::string, std::string>& context);

private:
    std::string getStatusMessage(int code);
    std::vector<std::string> cookies;
};

#endif // HTTPRESPONSE_HPP
