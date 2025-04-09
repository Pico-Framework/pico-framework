/**
 * @file HttpRequest.cpp
 * @author Ian Archbell
 * @brief Implementation of the HttpRequest class for parsing HTTP requests.
 *
 * Part of the PicoFramework HTTP server.
 * This module handles parsing the HTTP request line, headers, and body.
 * It also provides methods to access request parameters, cookies, and multipart data.
 * It uses the MultipartParser for handling multipart/form-data requests.
 *
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */
#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"
TRACE_INIT(HttpRequest)

#include "HttpRequest.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include "HttpServer.h"
#include <cstring>
#include <cstdio>
#include <unordered_map>
#include "lwip/sockets.h"
#include "utility.h"
#include "MultipartParser.h"
#include "url_utils.h"
#include "HttpParser.h"
#include "HttpClient.h"

#define BUFFER_SIZE 1460 // this is the standard MTU size

HttpResponse HttpRequest::get(const std::string& url) {
    setMethod("GET");
    setUri(url);
    return send();
}

HttpResponse HttpRequest::post(const std::string& url, const std::string& body) {
    setMethod("POST");
    setUri(url);
    setBody(body);
    return send();
}

HttpResponse HttpRequest::put(const std::string& url, const std::string& body) {
    setMethod("PUT");
    setUri(url);
    setBody(body);
    return send();
}

HttpResponse HttpRequest::del(const std::string& url) {
    setMethod("DELETE");
    setUri(url);
    return send();
}

HttpResponse HttpRequest::send() {
    HttpResponse response;
    HttpClient client;
    client.sendRequest(*this, response);
    return response;
}

/**
 * @brief Construct a HttpRequest object and parse the URL and headers.
 *
 * @param rawHeaders Raw header string.
 * @param reqMethod HTTP method (GET, POST, etc.).
 * @param reqPath HttpRequest URL path.
 */
HttpRequest::HttpRequest(const char *rawHeaders, const std::string &reqMethod, const std::string &reqPath)
    : method(reqMethod), path(reqPath)
{

    uri = reqPath; // Store the original URL
    // Parse the URL to separate path and query string
    size_t pos = uri.find('?');
    if (pos != std::string::npos)
    {
        path = uri.substr(0, pos);
        query = uri.substr(pos + 1);
    }
    else
    {
        path = uri;
        query = "";
    }
    parseHeaders(rawHeaders);
}

HttpRequest& HttpRequest::setBody(const std::string& b) {
    body = b;
    return *this;
}

void HttpRequest::parseHeaders(const char* raw) {
    headers = HttpParser::parseHeaders(raw);
}

/**
 * @brief Receive raw bytes from the client socket into a buffer.
 *
 * @param clientSocket The socket descriptor.
 * @param buffer The buffer to fill.
 * @param size Maximum size of the buffer.
 * @return int Number of bytes received, or -1 on error.
 */
int HttpRequest::receiveData(int clientSocket, char *buffer, int size)
{

    size_t bytesReceived = lwip_recv(clientSocket, buffer, size - 1, 0);
    if (bytesReceived < 0)
    {
        printf("Error receiving data from client: %zu\n", bytesReceived);
        return -1;
    }

    printf("Received %zu bytes\n", bytesReceived);
    printf("Buffer: %.*s\n", (int)bytesReceived, buffer);

    if (bytesReceived == 0)
    {
        printf("Client disconnected.\n");
        return -1;
    }

    buffer[bytesReceived] = '\0'; // Null-terminate the received data
    TRACE("Received %zu bytes\n", bytesReceived);
    TRACE("Buffer: %.*s\n", bytesReceived, buffer);
    return bytesReceived;
}

/**
 * @brief Extract HTTP method and path from request line.
 *
 * @param buffer The raw request buffer.
 * @param clientSocket The client socket (not used, included for consistency).
 * @param method Output buffer for method.
 * @param path Output buffer for path.
 * @return true on success, false on failure.
 */
bool HttpRequest::getMethodAndPath(char *buffer, int clientSocket, char *method, char *path)
{
    // Extract the HTTP method and path
    if (sscanf(buffer, "%s %s", method, path) != 2)
    {
        printf("Error parsing HTTP request method and path\n");
        return false;
    }
    return true;
}

/**
 * @brief Receive and parse a full HTTP request from a socket.
 *
 * @param clientSocket The client socket.
 * @return HttpRequest The fully parsed request object.
 */
HttpRequest HttpRequest::receive(int clientSocket)
{
    printf("Receiving request on socket %d\n", clientSocket);
    char buffer[BUFFER_SIZE];                             // Declare buffer size
    std::string body = "";                                // Initialize empty body
    std::map<std::string, std::string> headers; // Initialize empty headers
    char method[16] = {0};                                // Initialize method buffer
    char path[BUFFER_SIZE] = {0};                         // Initialize path buffer

    int bytesReceived = receiveData(clientSocket, (char *)&buffer, sizeof(buffer));
    if (bytesReceived == -1)
    {
        return HttpRequest("", "", ""); // Return empty HttpRequest on error
    }
    if (!getMethodAndPath((char *)&buffer, clientSocket, (char *)&method, (char *)&path))
    {
        return HttpRequest("", "", ""); // Return empty HttpRequest on error
    }

    // Identify the raw headers - look for the end of the headers (a double CRLF "\r\n\r\n")
    printf("Buffer received: %.*s\n", bytesReceived, buffer);
    size_t headerEnd = 0;
    while (headerEnd < bytesReceived)
    {
        if (buffer[headerEnd] == '\r' && buffer[headerEnd + 1] == '\n' &&
            buffer[headerEnd + 2] == '\r' && buffer[headerEnd + 3] == '\n')
        {
            headerEnd += 4; // Move past the double CRLF that indicates the end of the headers
            break;
        }
        headerEnd++;
    }
    printf("Raw headers length: %zu\n", headerEnd);

    // Create the request which will parse the headers
    HttpRequest request(buffer, std::string(method), std::string(path));

    // Get headers from the HttpRequest object
    headers = request.getHeaders(); // Retrieve headers from the HttpRequest object

    TRACE("Parsed headers:\n");
    for (const auto &header : headers)
    {
        TRACE("%s: %s\n", header.first.c_str(), header.second.c_str());
    }

    // Get the content length from the headers
    int contentLength = request.getContentLength();
    printf("Content-Length: %d\n", contentLength);

    // Handle the body only if contentLength is greater than 0
    if (contentLength > 0)
    {
        // First, handle any body data that might be in the first buffer
        size_t bodyReceived = bytesReceived - headerEnd;
        if (bodyReceived > 0)
        {
            body.append(buffer + headerEnd, bodyReceived);
        }
        request.setBody(body); // Set the body in the HttpRequest object

        // Check for multipart data and delegate to handler if necessary
        if (request.isMultipart())
        {
            TRACE("Multipart request detected\n");
            request.handle_multipart(clientSocket, request);
            TRACE("Multipart request handled\n");
            return request;
        }
        printf("Non-multipart request detected\n");

        // For non-multipart data, continue receiving the body data in chunks
        size_t bodyRemaining = contentLength - body.length();
        printf("Body remaining: %zu\n", bodyRemaining);
        while (bodyRemaining > 0)
        {
            size_t bytesToReceive = std::min(bodyRemaining, sizeof(buffer) - 1); // Limit to buffer size
            size_t chunkReceived = lwip_recv(clientSocket, buffer, bytesToReceive, 0);

            if (chunkReceived <= 0)
            {
                printf("Error receiving body data, bytes received: %zu\n", chunkReceived);
                return HttpRequest("", "", ""); // Return empty HttpRequest on error
            }

            body.append(buffer, chunkReceived); // Append received data to body
            bodyRemaining -= chunkReceived;     // Decrease remaining body size
        }
        printf("Body: %s\n", body.c_str());
    }
    printf("HttpRequest object constructed\n");
    return request; // Return the constructed HttpRequest object
}

/**
 * @brief Handle multipart/form-data content using MultipartParser.
 *
 * @param clientSocket The client socket.
 * @param req Reference to this request.
 * @return int 0 on success, -1 on failure.
 */
int HttpRequest::handle_multipart(int clientSocket, HttpRequest &req)
{
    MultipartParser parser(clientSocket, req);
    return parser.handleMultipart() ? 0 : -1;
}

/**
 * @brief Extract and return all cookies from the Cookie header.
 *
 * @return std::unordered_map<std::string, std::string> Map of cookie name-value pairs.
 */
std::unordered_map<std::string, std::string> HttpRequest::getCookies() const
{
    std::unordered_map<std::string, std::string> cookies;
    std::string cookieHeader = getHeader("cookie");
    std::istringstream stream(cookieHeader);
    std::string pair;
    while (std::getline(stream, pair, ';'))
    {
        size_t eq = pair.find('=');
        if (eq != std::string::npos)
        {
            std::string key = pair.substr(0, eq);
            std::string value = pair.substr(eq + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            value.erase(0, value.find_first_not_of(" \t"));
            cookies[key] = value;
        }
    }
    return cookies;
}

/**
 * @brief Get a specific cookie by name.
 *
 * @param name Cookie name.
 * @return std::string Value of the cookie or empty string.
 */
std::string HttpRequest::getCookie(const std::string &name) const
{
    auto cookies = getCookies();
    auto it = cookies.find(name);
    return (it != cookies.end()) ? it->second : "";
}

/**
 * @brief Parse the query string into a key-value map.
 *
 * @return Map of query parameters.
 */
std::unordered_map<std::string, std::string> HttpRequest::getQueryParams()
{
    return parseUrlEncoded(query);
}

/**
 * @brief Parse the request body as form-urlencoded content.
 *
 * @return Map of form parameters.
 */
std::unordered_map<std::string, std::string> HttpRequest::getFormParams()
{
    return parseUrlEncoded(body);
}

// ─────────────────────────────────────────────────────────────────────────────
// Fluent Builder Methods - some shared server-side
// ─────────────────────────────────────────────────────────────────────────────

HttpRequest HttpRequest::create() {
    return HttpRequest("", "", "");
}

HttpRequest& HttpRequest::setUri(const std::string& uri) {
    this->uri = uri;
    return *this;
}

HttpRequest& HttpRequest::setHost(const std::string& h) {
    host = h;
    return *this;
}

HttpRequest& HttpRequest::setProtocol(const std::string& p) {
    protocol = p;
    return *this;
}

HttpRequest& HttpRequest::setHeaders(const std::map<std::string, std::string>& headers) {
    for (const auto& [k, v] : headers) {
        this->headers[k] = v;
    }
    return *this;
}

HttpRequest& HttpRequest::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
    return *this;
}

HttpRequest& HttpRequest::setUserAgent(const std::string& userAgent) {
    headers["User-Agent"] = userAgent;
    return *this;
}

HttpRequest& HttpRequest::setAcceptEncoding(const std::string& encoding) {
    headers["Accept-Encoding"] = encoding;
    return *this;
}

#if defined(PICO_HTTP_ENABLE_STORAGE)
#include <fstream>
HttpRequest& HttpRequest::setBodyFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (file) {
        body.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    return *this;
}

bool HttpRequest::setRootCACertificateFromFile(const char* path) {
    std::string contents;
    if (!StorageManager::instance().readFileToString(path, contents)) {
        return false;
    }
    setRootCACertificate(contents);
    return true;
}
#endif
