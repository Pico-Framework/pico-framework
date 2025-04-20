/**
 * @file HttpResponse.cpp
 * @author Ian Archbell
 * @brief Implementation of the HttpResponse class for managing and sending HTTP responses.
 *
 * Part of the PicoFramework HTTP server.
 * This module handles constructing HTTP responses, setting headers, and sending
 * the response back to the client. It also supports setting content types,
 * content lengths, and handling different response statuses.
 *
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
#include "DebugTrace.h"
TRACE_INIT(HttpResponse)

#include "HttpResponse.h"
#include <sstream>
#include <cstring>
#include <lwip/sockets.h>
#include "utility.h"
#include "FrameworkView.h"

// ------------------------------------------------------------------------
// Constructor
// ------------------------------------------------------------------------

/**
 * @brief Construct a new HttpResponse object.
 * @param sock Socket descriptor for the client connection.
 */
HttpResponse::HttpResponse(Tcp* tcp)
    : tcp(tcp), status_code(200), headerSent(false)
{
}

// ------------------------------------------------------------------------
// Status and Header Management
// ------------------------------------------------------------------------

/**
 * @copydoc HttpResponse::status()
 */
HttpResponse &HttpResponse::status(int code)
{
    status_code = code;
    return *this;
}

/**
 * @copydoc HttpResponse::setStatus()
 */
HttpResponse& HttpResponse::setStatus(int code)
{
    status_code = code;
    return *this;
}

/**
 * @copydoc HttpResponse::set()
 */
HttpResponse& HttpResponse::set(const std::string &field, const std::string &value)
{
    headers[field] = value;
    return *this;
}

/**
 * @copydoc HttpResponse::setHeader()
 */
HttpResponse& HttpResponse::setHeader(const std::string &key, const std::string &value)
{
    headers[key] = value;
    return *this;
}

/**
 * @copydoc HttpResponse::setContentType()
 */
HttpResponse& HttpResponse::setContentType(const std::string &ct)
{
    headers["Content-Type"] = ct;
    return *this;
}

/**
 * @copydoc HttpResponse::setAuthorization()
 */
HttpResponse& HttpResponse::setAuthorization(const std::string &jwtToken)
{
    if (!jwtToken.empty())
    {
        headers["Authorization"] = "Bearer " + jwtToken;
    }
    return *this;
}

/**
 * @copydoc HttpResponse::getContentType()
 */
std::string HttpResponse::getContentType() const
{
    auto it = headers.find("Content-Type");
    return (it != headers.end()) ? it->second : "text/html";
}

/**
 * @copydoc HttpResponse::isHeaderSent()
 */
bool HttpResponse::isHeaderSent() const
{
    return headerSent;
}

/**
 * @copydoc HttpResponse::getSocket()
 */
int HttpResponse::getSocket() const
{
    return tcp->getSocketFd();
}

/**
 * @brief Return the standard HTTP status message for a code.
 * @param code HTTP status code.
 * @return Corresponding status message.
 */
std::string HttpResponse::getStatusMessage(int code)
{
    switch (code)
    {
    case 200:
        return "OK";
    case 401:
        return "Unauthorized";
    case 404:
        return "Not Found";
    case 500:
        return "Internal Server Error";
    default:
        return "";
    }
}

// ------------------------------------------------------------------------
// Cookie Support
// ------------------------------------------------------------------------

/**
 * @copydoc HttpResponse::setCookie()
 */
HttpResponse &HttpResponse::setCookie(const std::string &name, const std::string &value, const std::string &options)
{
    std::ostringstream cookie;
    cookie << name << "=" << value;
    if (!options.empty())
    {
        cookie << "; " << options;
    }
    cookies.push_back(cookie.str());
    return *this;
}

/**
 * @copydoc HttpResponse::clearCookie()
 */
HttpResponse &HttpResponse::clearCookie(const std::string &name, const std::string &options)
{
    std::ostringstream cookie;
    cookie << name << "=; Max-Age=0";
    if (!options.empty())
    {
        cookie << "; " << options;
    }
    cookies.push_back(cookie.str());
    return *this;
}

// ------------------------------------------------------------------------
// Body and Streaming
// ------------------------------------------------------------------------

/**
 * @copydoc HttpResponse::send()
 */
void HttpResponse::send(const std::string &body)
{
    TRACE("HttpResponse::send()\n");
    if (!headerSent)
    {
        if (headers.find("Content-Length") == headers.end())
        {
            headers["Content-Length"] = std::to_string(body.size());
        }
        if (headers.find("Content-Type") == headers.end())
        {
            headers["Content-Type"] = "text/html";
        }

        std::ostringstream resp;
        resp << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
        for (auto &h : headers)
        {
            resp << h.first << ": " << h.second << "\r\n";
        }
        for (const auto &cookie : cookies)
        {
            resp << "Set-Cookie: " << cookie << "\r\n";
        }
        resp << "\r\n";

        std::string header_str = resp.str();
        tcp->send(header_str.c_str(), header_str.size());
        headerSent = true;
    }

    tcp->send(body.data(), body.size());
    TRACE("HttpResponse::send() completed\n");
}

/**
 * @copydoc HttpResponse::sendHeaders()
 */
void HttpResponse::sendHeaders()
{
    if (!headerSent)
    {
        std::ostringstream resp;
        resp << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";

        for (auto &h : headers)
        {
            resp << h.first << ": " << h.second << "\r\n";
        }
        for (const auto &cookie : cookies)
        {
            resp << "Set-Cookie: " << cookie << "\r\n";
        }
        if (headers.find("Connection") == headers.end())
        {
            resp << "Connection: close\r\n";
        }
        resp << "\r\n";

        std::string header_str = resp.str();
        tcp->send(header_str.c_str(), header_str.size()); 
        headerSent = true;
    }
}

/**
 * @copydoc HttpResponse::start()
 */
void HttpResponse::start(int code, size_t contentLength, const std::string &contentType, const std::string &contentEncoding)
{
    status_code = code;
    headers["Content-Length"] = std::to_string(contentLength);
    headers["Content-Type"] = contentType;
    if (!contentEncoding.empty())
    {
        headers["Content-Encoding"] = contentEncoding;
    }

    std::ostringstream resp;
    resp << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
    for (auto &h : headers)
    {
        resp << h.first << ": " << h.second << "\r\n";
    }
    for (const auto &cookie : cookies)
    {
        resp << "Set-Cookie: " << cookie << "\r\n";
    }
    resp << "\r\n";

    std::string header_str = resp.str();
    tcp->send(header_str.c_str(), header_str.size());
    headerSent = true;
}

/**
 * @copydoc HttpResponse::writeChunk()
 */
void HttpResponse::writeChunk(const char *data, size_t length)
{
    if (!headerSent)
    {
        printf("Error: writeChunk called before start()\n");
        return;
    }

    ssize_t err = tcp->send(data, length); 
    if (err < 0)
    {
        printf("Error sending chunk: %zu\n", err);
        printf("Error: %s\n", strerror(errno));
    }
}

/**
 * @copydoc HttpResponse::finish()
 */
void HttpResponse::finish()
{
    // Placeholder for future expansion (e.g., chunked transfer end).
}

// ------------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------------

/**
 * @copydoc HttpResponse::sendUnauthorized()
 */
void HttpResponse::sendUnauthorized()
{
    this->status(401)
        .set("Content-Type", "application/json")
        .send("{\"error\": \"Unauthorized\"}");
}

/**
 * @copydoc HttpResponse::sendNotFound()
 */
void HttpResponse::sendNotFound()
{
    return status(404)
        .setContentType("application/json")
        .send(R"({"error": "Not Found"})");
}

/**
 * @copydoc HttpResponse::endServerError()
 */
void HttpResponse::endServerError(const std::string &message)
{
    return status(500)
        .setContentType("application/json")
        .send("{\"error\": \"" + message + "\"}");
}

/**
 * @copydoc HttpResponse::json()
 */
HttpResponse &HttpResponse::json(const std::string &body)
{
    this->set("Content-Type", "application/json")
        .send(body);
    return *this;
}
// send a json object
HttpResponse &HttpResponse::json(const nlohmann::json &jsonObj)
{
    return json(jsonObj.dump()); // dump() creates a compact string
}

HttpResponse &HttpResponse::jsonFormatted(const nlohmann::json &jsonObj)
{
    return json(jsonObj.dump(2)); // dump() creates a compact string
}

/**
 * @copydoc HttpResponse::text()
 */
HttpResponse &HttpResponse::text(const std::string &body)
{
    this->set("Content-Type", "text/plain")
        .send(body);
    return *this;
}

HttpResponse &HttpResponse::redirect(const std::string &url, int code)
{
    this->status(code)
        .set("Location", url)
        .send("");
    return *this;
}

/**
 * @copydoc HttpResponse::renderTemplate()
 */
std::string HttpResponse::renderTemplate(const std::string &tpl, const std::map<std::string, std::string> &context)
{
    std::string result = tpl;
    for (const auto &[key, value] : context)
    {
        std::string placeholder = "{{" + key + "}}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos)
        {
            result.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }
    return result;
}


// ------------------------------------------------------------------------
// ─────                 View related                                 ─────
// ------------------------------------------------------------------------

void HttpResponse::renderView(const std::string& filename, const std::map<std::string, std::string>& context) {
    std::string fullPath = "/www/" + filename;
    std::string html = FrameworkView::render(fullPath, context);
    send(html, "text/html");  
}

void HttpResponse::send(const std::string& body, const std::string& contentType) {
    setHeader("Content-Type", contentType);
    send(body);
}


HttpResponse& HttpResponse::setBody(const std::string& body) {
    this->body = body;
    return *this;
}

void HttpResponse::reset() {
    status_code = 0;
    headers.clear();
    body.clear();
}

#if defined(PICO_HTTP_ENABLE_STORAGE)
bool HttpResponse::saveFile(const char* path) const {
    return StorageManager::instance().writeFile(path, body_);
}
#endif

