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

#include "http/HttpRequest.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <unordered_map>
// #include <lwip/sockets.h>

#include "http/HttpServer.h"
#include "utility/utility.h"
#include "http/MultipartParser.h"
#include "http/url_utils.h"
#include "http/HttpParser.h"
#include "network/Tcp.h"

#ifdef PICO_HTTP_ENABLE_HTTP_CLIENT
#include "http/HttpClient.h"
#endif // PICO_HTTP_ENABLE_HTTP_CLIENT

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
#ifdef PICO_HTTP_ENABLE_HTTP_CLIENT
    HttpResponse response;
    HttpClient client;
    client.sendRequest(*this, response);
    return response;
#else
    // If HTTP client is not enabled, return an empty response
    return HttpResponse(nullptr);
#endif
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

/**
 * @brief Receive raw bytes from the client socket into a buffer.
 *
 * @param Tcp* The TCP connection to read from.
 * @return HttpRequest Parsed HttpRequest object containing method, path, headers, and body.
 * @note This function reads the request line, headers, and body from the TCP socket.
 * It handles both multipart and non-multipart requests, parsing headers and body accordingly.
 */

HttpRequest HttpRequest::receive(Tcp *tcp)
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
    buffer[bytesReceived] = '\0'; // Ensure null-termination for strstr() and strtok()

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
    std::string rawHeaders(buffer, headerEnd);
    TRACE("Raw headers: %s\n", rawHeaders.c_str());
    HttpRequest request(tcp, rawHeaders, std::string(method), std::string(path));

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
            // HttpResponse response;
            // request.handle_multipart(response);  // tcp is already stored in request
            TRACE("Multipart request created\n");
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

            // Enforce maximum body length
            size_t currentSize = body.size();
            if (currentSize >= MAX_HTTP_BODY_LENGTH)
            {
                TRACE("Body length exceeds max allowed (%d bytes). Truncating.\n", MAX_HTTP_BODY_LENGTH);
                break;
            }

            size_t allowed = MAX_HTTP_BODY_LENGTH - currentSize;
            size_t toAppend = std::min(static_cast<size_t>(chunkReceived), allowed);
            body.append(buffer, toAppend);

            if (toAppend < static_cast<size_t>(chunkReceived))
            {
                TRACE("Chunk truncated due to body size limit.\n");
                request.markBodyTruncated();
                break;
            }

            bodyRemaining -= chunkReceived;
        }
        request.setBody(body);
        TRACE("Final body length: %zu\n", body.length());
        TRACE("HttpRequest object constructed\n");
    }
    return request;
}

/**
 * @brief Handle multipart/form-data content using MultipartParser.
 *
 * @param clientSocket The client socket.
 * @param req Reference to this request.
 * @return int 0 on success, -1 on failure.
 */
int HttpRequest::handle_multipart(HttpResponse &res)
{
    MultipartParser parser;
    return parser.handleMultipart(*this, res) ? 0 : -1;
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
HttpRequest &HttpRequest::setUri(const std::string &uri)
{
    const auto proto_end = uri.find("://");
    if (proto_end == std::string::npos)
    {
        // Relative URI — just set it, do not overwrite protocol or host
        this->uri = uri;
        return *this;
    }

    // Full URL — parse into protocol, host, and path
    protocol = uri.substr(0, proto_end);
    std::string rest = uri.substr(proto_end + 3); // skip "://"

    const auto path_start = rest.find('/');
    if (path_start == std::string::npos)
    {
        host = rest;
        this->uri = "/";
    }
    else
    {
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
