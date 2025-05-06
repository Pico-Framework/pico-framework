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

#define BUFFER_SIZE 256 

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
bool HttpRequest::getMethodAndPath(const std::string& rawHeaders, std::string& method, std::string& path)
{
    printf("Parsing request data: %s\n", rawHeaders.c_str());
    std::istringstream stream(rawHeaders);
    std::string requestLine;
    if (!std::getline(stream, requestLine)) {
        printf("Failed to read request line\n");
        return false;
    }
    
    std::istringstream lineStream(requestLine);
    if (!(lineStream >> method >> path)) {
        printf("Error parsing HTTP request method and path\n");
        return false;
    }
    return true;
}

constexpr size_t MAX_HEADER_BYTES = 4096; // limit to prevent runaway headers if malformed

std::optional<std::pair<std::string, std::string>> HttpRequest::receiveUntilHeadersComplete(Tcp* conn) {
    std::string requestText;
    char buffer[HTTP_BUFFER_SIZE];

    while (true) {
        int received = conn->recv(buffer, sizeof(buffer));
        if (received <= 0) {
            printf("[HttpRequest] Failed to receive header bytes\n");
            return std::nullopt;
        }

        requestText.append(buffer, received);

        size_t headerEnd = requestText.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            size_t bodyStart = headerEnd + 4;
            std::string headers = requestText.substr(0, headerEnd);
            std::string leftover = (bodyStart < requestText.size())
                ? requestText.substr(bodyStart)
                : "";
            return std::make_pair(std::move(headers), std::move(leftover));
        }

        if (requestText.size() > MAX_HEADER_BYTES) {
            printf("[HttpRequest] Headers exceeded %zu bytes, rejecting\n", MAX_HEADER_BYTES);
            return std::nullopt;
        }
    }
}

bool HttpRequest::appendRemainingBody(int expectedLength) {
    size_t remaining = expectedLength - body.size();
    char buffer[HTTP_BUFFER_SIZE];

    while (remaining > 0) {
        size_t toRead = std::min(remaining, sizeof(buffer));
        int received = tcp->recv(buffer, toRead);
        if (received <= 0) {
            printf("Error receiving body chunk\n");
            return false;
        }
        size_t currentSize = body.size();
        if (currentSize >= MAX_HTTP_BODY_LENGTH) {
            TRACE("Body exceeds max length. Truncating.\n");
            markBodyTruncated();
            break;
        }
        size_t allowed = MAX_HTTP_BODY_LENGTH - currentSize;
        size_t toAppend = std::min(static_cast<size_t>(received), allowed);
        body.append(buffer, toAppend);

        if (toAppend < static_cast<size_t>(received)) {
            TRACE("Body chunk truncated due to size limit.\n");
            markBodyTruncated();
            break;
        }
        remaining -= received;
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
    std::string method = {0};                      // Initialize method buffer
    std::string path = {0};               // Initialize path buffer

    auto result = receiveUntilHeadersComplete(tcp);
    if (!result) {
        return HttpRequest("", "", "");
    }
    
    const auto& [rawHeaders, initialBody] = *result;

    if (!getMethodAndPath(rawHeaders, method, path)) {
        return HttpRequest("", "", "");
    }

    // Create the request which will parse the headers
    TRACE("Raw headers: %s\n", rawHeaders.c_str());

    HttpRequest request(tcp, rawHeaders, std::string(method), std::string(path));
    request.setBody(initialBody); // Set the initial body if any

    // NOW split path and query
    std::string cleanPath = path;
    std::string queryString = "";

    size_t qpos = cleanPath.find('?');
    if (qpos != std::string::npos)
    {
        queryString = cleanPath.substr(qpos + 1);
        cleanPath = cleanPath.substr(0, qpos);
    }
    request.setPath(cleanPath); // Set the cleaned path as the URI
    request.setQueryString(queryString); // Set the query string

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
        TRACE("Content-Length is greater than 0\n");
        if (request.isMultipart())
        {
            TRACE("Multipart request detected\n");
            return request;
        }

        TRACE("Non-multipart request detected\n");

        request.appendRemainingBody(contentLength);
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
const std::unordered_map<std::string, std::string> HttpRequest::getCookies() const
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
const std::string HttpRequest::getCookie(const std::string &name) const
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
const std::unordered_multimap<std::string, std::string> HttpRequest::getQueryParams()
{
    return parseUrlEncoded(query);
}

/**
 * @brief Parse the request body as form-urlencoded content.
 *
 * @return Map of form parameters.
 */
const std::unordered_multimap<std::string, std::string> HttpRequest::getFormParams()
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


HttpRequest& HttpRequest::toFile(const std::string& path) {
    this->outputFilePath = path;
    return *this;
}

bool HttpRequest::wantsToFile() const {
    return !outputFilePath.empty();
}

std::string HttpRequest::getOutputFilePath() const {
    return outputFilePath;
}
