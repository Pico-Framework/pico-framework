/**
 * @file HttpRequest.h
 * @author Ian Archbell
 * @brief Defines the HttpRequest class for handling HTTP requests: headers, method, path,
 *        query string, cookies, and body (including multipart forms).
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 *
 * @copyright Copyright (c) 2025
 */

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
#pragma once

#include "utility.h"
#include "HttpResponse.h"
#include <string>
#include <map>
#include <iostream>
// #include "TcpConnectionSocket.h"

class Router; ///< Forward declaration for potential routing needs

/**
 * @class HttpRequest
 * @brief Represents a parsed HTTP request, providing access to headers, method, path, body, etc.
 */
class HttpRequest
{
public:
    // ─────────────────────────────────────────────────────────────────────────────
    // Constructors
    // ─────────────────────────────────────────────────────────────────────────────

    /**
     * @brief Construct a HttpRequest from raw headers, method, and path.
     * @param rawHeaders Raw HTTP headers as a C-string.
     * @param reqMethod HTTP method.
     * @param reqPath HttpRequest path.
     */
    HttpRequest(const char *rawHeaders, const std::string &reqMethod, const std::string &reqPath);

    // used by fluent builder - client side
    HttpRequest(const std::string& raw, const std::string& method, const std::string& path)
    : method(method), path(path), uri(path) {}

    HttpRequest() = default;
    HttpRequest(const HttpRequest&) = default;

    // ─────────────────────────────────────────────────────────────────────────────
    // Store a CA Root Certificate
    // ─────────────────────────────────────────────────────────────────────────────

    /**
     * @brief Set the root CA certificate to use for TLS.
     * 
     * @param certData PEM-encoded certificate.
     */
    void setRootCACertificate(const std::string& certData);

#if defined(PICO_HTTP_ENABLE_STORAGE)
    /**
     * @brief Load and set the root CA certificate from a file using StorageManager.
     * 
     * @param path Path to the certificate file.
     * @return true if loaded successfully, false otherwise.
     */
    bool setRootCACertificateFromFile(const char* path);
#endif


    // ─────────────────────────────────────────────────────────────────────────────
    // Easy access methods for sending requests
    // ─────────────────────────────────────────────────────────────────────────────
    
    /**
     * @brief Send the request and return the response.
     * @return HttpResponse object containing the response.
     */
    HttpResponse send();

    /**
     * @brief Send a GETPOST/PUT/DEL request.
     * @param url The URL to send the request to.
     * @return HttpResponse object containing the response.
     */
    HttpResponse get(const std::string& url);
    HttpResponse post(const std::string& url, const std::string& body);
    HttpResponse put(const std::string& url, const std::string& body);
    HttpResponse del(const std::string& url);


    // ─────────────────────────────────────────────────────────────────────────────
    // Header Accessors
    // ─────────────────────────────────────────────────────────────────────────────

    /**
     * @brief Get a specific header field (case-insensitive).
     * @param field Header field name.
     * @return The header value or empty string if not found.
     */
    std::string getHeader(const std::string &field) const
    {
        auto it = headers.find(toLower(field));
        return (it != headers.end()) ? it->second : "";
    }

    /**
     * @brief Get all request headers.
     * @return Map of header key-value pairs.
     */
    std::map<std::string, std::string> getHeaders() const
    {
        return headers;
    }

    /**
     * @brief Print all headers to the standard output.
     */
    void printHeaders() const
    {
        for (const auto &header : headers)
        {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }

    /**
     * @brief Set the position marking the end of headers.
     * @param end Offset into the raw request.
     */
    void setHeaderEnd(size_t end)
    {
        headerEnd = end;
    }

    /**
     * @brief Get the header end offset (used for body parsing).
     * @return Byte offset.
     */
    size_t getHeaderEnd()
    {
        return headerEnd;
    }

    /**
     * @brief Get the Host header value.
     */
    std::string getHost() const
    {
        return host;
    }

    std::string getProtocol() const
    {
        return protocol;
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // Content Type / Body Type Checkers
    // ─────────────────────────────────────────────────────────────────────────────

    /**
     * @brief Check if content-type is application/x-www-form-urlencoded.
     */
    bool isFormUrlEncoded() const
    {
        auto it = headers.find("content-type");
        return it != headers.end() && it->second.find("application/x-www-form-urlencoded") != std::string::npos;
    }

    /**
     * @brief Check if content-type is application/json.
     */
    bool isJson() const
    {
        auto it = headers.find("content-type");
        return it != headers.end() && it->second.find("application/json") != std::string::npos;
    }

    /**
     * @brief Get the raw Content-Type string.
     */
    std::string getContentType() const
    {
        return getHeader("content-type");
    }

    /**
     * @brief Get the boundary string (for multipart/form-data).
     * @return Boundary string or empty string if not present.
     */
    std::string getBoundary() const
    {
        auto contentType = getContentType();
        auto boundaryPos = contentType.find("boundary=");
        if (boundaryPos != std::string::npos)
        {
            return contentType.substr(boundaryPos + 9);
        }
        return "";
    }

    /**
     * @brief Check whether the request is multipart/form-data.
     */
    bool isMultipart() const
    {
        auto contentType = getHeader("content-type");
        return contentType.find("multipart/form-data") != std::string::npos;
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // Body Accessors
    // ─────────────────────────────────────────────────────────────────────────────

    /**
     * @brief Get the request body (copy).
     */
    std::string getBody() const
    {
        return body;
    }

    /**
     * @brief Set the body of the request.
     * @param aBody The full request body content.
     */
    HttpRequest &setBody(const std::string &body); // Enables chaining

    /**
     * @brief Get the Content-Length header as integer.
     * @return Content length in bytes, or 0 if absent.
     */
    int getContentLength() const
    {
        std::string content_length_str = getHeader("content-length");
        int contentLength = 0;
        if (!content_length_str.empty())
        {
            contentLength = std::stoi(content_length_str);
        }
        return contentLength;
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // Method / URL / Path Accessors
    // ─────────────────────────────────────────────────────────────────────────────

    /**
     * @brief Set the HTTP method (e.g., GET, POST).
     */
    HttpRequest& setMethod(const std::string &method)
    {
        this->method = method;
        return *this;
    }

    /**
     * @brief Set the request path.
     */
    HttpRequest& setPath(const std::string &path)
    {
        this->path = path;
        return *this;
    }

    /**
     * @brief Get the HTTP method.
     */
    std::string getMethod() const
    {
        return method;
    }

    /**
     * @brief Get the parsed request path (without query string).
     */
    std::string getPath() const
    {
        return path;
    }

    /**
     * @brief Get the original URL from the request line.
     */
    std::string getUri() const
    {
        return uri;
    }

    /**
     * @brief Get the parsed query string from the URL.
     */
    std::string getQuery() const
    {
        return query;
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // Client Info
    // ─────────────────────────────────────────────────────────────────────────────

    /**
     * @brief Set the client IP address.
     * @param ip The IP as a string.
     */
    void setClientIp(const std::string &ip)
    {
        clientIp = ip;
    }

    /**
     * @brief Get the client IP address.
     */
    std::string getClientIp() const
    {
        return clientIp;
    }

    // ─────────────────────────────────────────────────────────────────────────────
    // Cookie and Parameter Access
    // ─────────────────────────────────────────────────────────────────────────────

    /**
     * @brief Get all parsed cookies.
     * @return A map of cookie names to values.
     */
    std::unordered_map<std::string, std::string> getCookies() const;

    /**
     * @brief Get a specific cookie value.
     * @param name Cookie name.
     * @return Cookie value or empty string.
     */
    std::string getCookie(const std::string &name) const;

    /**
     * @brief Get parsed query string parameters.
     */
    std::unordered_map<std::string, std::string> getQueryParams();

    /**
     * @brief Get parsed form fields (application/x-www-form-urlencoded).
     */
    std::unordered_map<std::string, std::string> getFormParams();

    // ─────────────────────────────────────────────────────────────────────────────
    // Socket-Based Helpers (Static)
    // ─────────────────────────────────────────────────────────────────────────────

    /**
     * @brief Receive and parse an HTTP request from a socket.
     * @param clientSocket The socket file descriptor.
     * @return A fully populated HttpRequest object.
     */
    static HttpRequest receive(int clientSocket);

    /**
     * @brief Receive raw data from socket into buffer.
     * @param clientSocket The socket file descriptor.
     * @param buffer Target buffer.
     * @param size Size of the buffer.
     * @return Number of bytes received, or -1 on error.
     */
    static int receiveData(int clientSocket, char *buffer, int size);

    /**
     * @brief Parse the HTTP method and path from the first request line.
     * @param buffer Raw request buffer.
     * @param clientSocket The socket (for logging/debug).
     * @param method Output buffer for method.
     * @param path Output buffer for path.
     * @return True on success.
     */
    static bool getMethodAndPath(char *buffer, int clientSocket, char *method, char *path);

    // ─────────────────────────────────────────────────────────────────────────────
    // Multipart Upload Handling
    // ─────────────────────────────────────────────────────────────────────────────

    /**
     * @brief Handle multipart/form-data uploads.
     * @param new_sock The socket.
     * @param req Reference to this request.
     * @return 0 on success, -1 on failure.
     */
    int handle_multipart(int new_sock, HttpRequest &req);

    // ─────────────────────────────────────────────────────────────────────────────
    // Fluent Builder Methods (Client Usage)
    // ─────────────────────────────────────────────────────────────────────────────

    static HttpRequest create(); // static builder factory
    HttpRequest &setUri(const std::string &uri);
    HttpRequest &setHost(const std::string &host);
    HttpRequest &setProtocol(const std::string &protocol);
    HttpRequest &setHeaders(const std::map<std::string, std::string> &headers);
    HttpRequest &setHeader(const std::string &key, const std::string &value);
    HttpRequest &setUserAgent(const std::string &userAgent);
    HttpRequest &setAcceptEncoding(const std::string &encoding);

    /**
     * @brief Get the root CA certificate string, if set.
     * 
     * @return const std::string& PEM-encoded root certificate.
     */
    const std::string& getRootCACertificate() const{
        return rootCACertificate;
    }

    #if defined(PICO_HTTP_ENABLE_STORAGE)
    HttpRequest& setBodyFromFile(const std::string& path);
    #endif

private:
    void parseHeaders(const char *raw);

    std::string clientIp;
    std::string method;
    std::string uri;
    std::string path;
    std::string query;
    std::string host;
    std::string protocol;
    std::map<std::string, std::string> headers;  // optional
    std::string body;
    std::string rootCACertificate;
    size_t headerEnd = 0;
};

#endif // HTTPREQUEST_H
