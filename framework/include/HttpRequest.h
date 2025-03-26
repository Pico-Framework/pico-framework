#ifndef REQUEST_HPP
#define REQUEST_HPP
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

    static Request receive(int clientSocket);
    static int receiveData(int clientSocket, char* buffer, int size);
    static bool getMethodAndPath(char* buffer, int clientSocket, char* method, char* path);

    // multipart handling
    int handle_multipart(int new_sock, Request& req); 

private:
    void parseHeaders(const char* raw);
    
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::string body;  // Internal storage for the body    
    size_t headerEnd = 0;


};

#endif // REQUEST_HPP
