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
#include "Tcp.h"

#define BUFFER_SIZE 1460 // this is the standard MTU size

HttpResponse HttpRequest::get()
{
    setMethod("GET");
    return send();
}

HttpResponse HttpRequest::get(const std::string &url)
{
    setMethod("GET");
    setUri(url);
    return send();
}

HttpResponse HttpRequest::post()
{
    setMethod("POST");
    return send();
}

HttpResponse HttpRequest::put()
{
    setMethod("PUT");
    return send();
}

HttpResponse HttpRequest::del()
{
    setMethod("DELETE");
    return send();
}

HttpResponse HttpRequest::post(const std::string &url, const std::string &body)
{
    setMethod("POST");
    setUri(url);
    setBody(body);
    return send();
}

HttpResponse HttpRequest::put(const std::string &url, const std::string &body)
{
    setMethod("PUT");
    setUri(url);
    setBody(body);
    return send();
}

HttpResponse HttpRequest::del(const std::string &url)
{
    setMethod("DELETE");
    setUri(url);
    return send();
}

HttpResponse HttpRequest::send()
{
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

HttpRequest &HttpRequest::setBody(const std::string &b)
{
    body = b;
    return *this;
}

void HttpRequest::parseHeaders(const char *raw)
{
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

/**
 * @brief Extract HTTP method and path from request line.
 *
 * @param buffer The raw request buffer.
 * @param clientSocket The client socket (not used, included for consistency).
 * @param method Output buffer for method.
 * @param path Output buffer for path.
 * @return true on success, false on failure.
 */
bool HttpRequest::getMethodAndPath(char *buffer, char *method, char *path)
{
    // Extract the HTTP method and path
    if (sscanf(buffer, "%s %s", method, path) != 2)
    {
        printf("Error parsing HTTP request method and path\n");
        return false;
    }
    return true;
}

HttpRequest HttpRequest::receive(Tcp* tcp)
{
    TRACE("Receiving request on socket %d\n", tcp->getSocketFd());

    char buffer[BUFFER_SIZE];                   // Declare buffer size
    std::string body = "";                      // Initialize empty body
    std::map<std::string, std::string> headers; // Initialize empty headers
    char method[16] = {0};                      // Initialize method buffer
    char path[BUFFER_SIZE] = {0};               // Initialize path buffer

    int bytesReceived = tcp->recv(buffer, sizeof(buffer));
    if (bytesReceived <= 0)
    {
        return HttpRequest("", "", ""); // Return empty HttpRequest on error
    }

    if (!getMethodAndPath(buffer, method, path))
    {
        return HttpRequest("", "", ""); // Return empty HttpRequest on error
    }

    // Identify the raw headers - look for the end of the headers (a double CRLF "\r\n\r\n")
    TRACE("Buffer received: %.*s\n", bytesReceived, buffer);
    size_t headerEnd = 0;
    while (headerEnd < static_cast<size_t>(bytesReceived))
    {
        if (buffer[headerEnd] == '\r' && buffer[headerEnd + 1] == '\n' &&
            buffer[headerEnd + 2] == '\r' && buffer[headerEnd + 3] == '\n')
        {
            headerEnd += 4; // Move past the double CRLF that indicates the end of the headers
            break;
        }
        headerEnd++;
    }
    TRACE("Raw headers length: %zu\n", headerEnd);

    // Create the request which will parse the headers
    HttpRequest request(tcp, buffer, std::string(method), std::string(path));

    // Get headers from the HttpRequest object
    headers = request.getHeaders();

    TRACE("Parsed headers:\n");
    for (const auto &header : headers)
    {
        TRACE("%s: %s\n", header.first.c_str(), header.second.c_str());
    }

    int contentLength = request.getContentLength();
    TRACE("Content-Length: %d\n", contentLength);

    if (contentLength > 0)
    {
        size_t bodyReceived = bytesReceived - headerEnd;
        if (bodyReceived > 0)
        {
            body.append(buffer + headerEnd, bodyReceived);
        }
        request.setBody(body);

        if (request.isMultipart())
        {
            TRACE("Multipart request detected\n");
            request.handle_multipart(request);  // tcp is already stored in request
            TRACE("Multipart request handled\n");
            return request;
        }

        TRACE("Non-multipart request detected\n");

        size_t bodyRemaining = contentLength - body.length();
        TRACE("Body remaining: %zu\n", bodyRemaining);
        while (bodyRemaining > 0)
        {
            size_t bytesToReceive = std::min(bodyRemaining, sizeof(buffer) - 1);
            int chunkReceived = tcp->recv(buffer, bytesToReceive);
            if (chunkReceived <= 0)
            {
                printf("Error receiving body data, bytes received: %d\n", chunkReceived);
                return HttpRequest("", "", "");
            }

            body.append(buffer, chunkReceived);
            bodyRemaining -= chunkReceived;
        }

        printf("Body: %s\n", body.c_str());
    }

    TRACE("HttpRequest object constructed\n");
    return request;
}


/**
 * @brief Handle multipart/form-data content using MultipartParser.
 *
 * @param clientSocket The client socket.
 * @param req Reference to this request.
 * @return int 0 on success, -1 on failure.
 */
int HttpRequest::handle_multipart(HttpRequest &req)
{
    MultipartParser parser(req);
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

HttpRequest HttpRequest::create()
{
    return HttpRequest("", "", "");
}

/**
 * @brief Set the URI for the request.
 * Parses the uri into protocol, host, and path. But not if it is just a /path
 * @param uri The URI to set.
 * @return Reference to this request.
 */
HttpRequest& HttpRequest::setUri(const std::string& uri) {
    const auto proto_end = uri.find("://");
    if (proto_end == std::string::npos) {
        // Relative URI — just set it, do not overwrite protocol or host
        this->uri = uri;
        return *this;
    }

    // Full URL — parse into protocol, host, and path
    protocol = uri.substr(0, proto_end);
    std::string rest = uri.substr(proto_end + 3); // skip "://"

    const auto path_start = rest.find('/');
    if (path_start == std::string::npos) {
        host = rest;
        this->uri = "/";
    } else {
        host = rest.substr(0, path_start);
        this->uri = rest.substr(path_start);
    }

    return *this;
}

HttpRequest &HttpRequest::setHost(const std::string &h)
{
    host = h;
    return *this;
}

HttpRequest &HttpRequest::setProtocol(const std::string &p)
{
    protocol = p;
    return *this;
}

HttpRequest &HttpRequest::setHeaders(const std::map<std::string, std::string> &headers)
{
    for (const auto &[k, v] : headers)
    {
        this->headers[k] = v;
    }
    return *this;
}

HttpRequest &HttpRequest::setHeader(const std::string &key, const std::string &value)
{
    headers[key] = value;
    return *this;
}

HttpRequest &HttpRequest::setUserAgent(const std::string &userAgent)
{
    headers["User-Agent"] = userAgent;
    return *this;
}

HttpRequest &HttpRequest::setAcceptEncoding(const std::string &encoding)
{
    headers["Accept-Encoding"] = encoding;
    return *this;
}
HttpRequest &HttpRequest::setRootCACertificate(const std::string &certData)
{
    rootCACertificate = certData;
    return *this;
}

#if defined(PICO_HTTP_ENABLE_STORAGE)
#include <fstream>
HttpRequest &HttpRequest::setBodyFromFile(const std::string &path)
{
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (file)
    {
        body.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    return *this;
}

bool HttpRequest::setRootCACertificateFromFile(const char *path)
{
    std::string contents;
    if (!StorageManager::instance().readFileToString(path, contents))
    {
        return false;
    }
    setRootCACertificate(contents);
    return true;
}
#endif
