/**
 * @file HttpResponse.cpp
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "HttpResponse.h" 
#include <sstream>
#include <lwip/sockets.h>
#include <cstring>
#include "utility.h"

// Immediately after lwip includes:
#ifdef send
#undef send
#endif

//#define TRACE_ON

Response::Response(int sock)
    : sock(sock), status_code(200), headerSent(false)
{}

Response& Response::status(int code) {
    status_code = code;
    return *this;
}

Response& Response::set(const std::string &field, const std::string &value) {
    headers[field] = value;
    return *this;
}

// Method to optionally set Authorization header (JWT)
Response& Response::setAuthorization(const std::string &jwtToken) {
    if (!jwtToken.empty()) {
        headers["Authorization"] = "Bearer " + jwtToken;  // Add Authorization header
    }
    return *this;  // Allow chaining
}

int Response::getSocket() const {
    return sock;
}

std::string Response::getStatusMessage(int code) {
    switch (code) {
        case 200: return "OK";
        case 401: return "Unauthorized";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default:  return "";
    }
}

// The old single-shot send
void Response::send(const std::string &body) {
    if (!headerSent) {
        // build default headers if not provided
        if (headers.find("Content-Length") == headers.end()) {
            headers["Content-Length"] = std::to_string(body.size());
        }
        if (headers.find("Content-Type") == headers.end()) {
            headers["Content-Type"] = "text/html";
        }
        
        // send initial headers
        std::ostringstream resp;
        resp << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
        for (auto &h : headers) {
            resp << h.first << ": " << h.second << "\r\n";
        }
        for (const auto& cookie : cookies) {
            resp << "Set-Cookie: " << cookie << "\r\n";
        }

        resp << "\r\n"; // end of headers

        std::string header_str = resp.str();
        lwip_send(sock, header_str.c_str(), header_str.size(), 0);

        headerSent = true;
    }
    // now send the body
    lwip_send(sock, body.data(), body.size(), 0);
}

void Response::sendHeaders() {
    if (!headerSent) {
        std::ostringstream resp;
        resp << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
        // Output all custom headers:
        for (auto &h : headers) {
            resp << h.first << ": " << h.second << "\r\n";
        }
        for (const auto& cookie : cookies) {
            resp << "Set-Cookie: " << cookie << "\r\n";
        }
        // Add a Connection header if not already present:
        if (headers.find("Connection") == headers.end()) {
            resp << "Connection: close\r\n";
        }
        resp << "\r\n";  // End of headers
        std::string header_str = resp.str();
        lwip_send(sock, header_str.c_str(), header_str.size(), 0);
        headerSent = true;
    }
}



// 1) start(...): Send initial header but no body yet
void Response::start(int code, size_t contentLength, const std::string &contentType)
{
    status_code = code;
    headers["Content-Length"] = std::to_string(contentLength);
    headers["Content-Type"] = contentType;

    // Build & send the initial headers
    std::ostringstream resp;
    resp << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
    for (auto &h : headers) {
        resp << h.first << ": " << h.second << "\r\n";
    }
    for (const auto& cookie : cookies) {
        resp << "Set-Cookie: " << cookie << "\r\n";
    }

    resp << "\r\n"; // end of headers

    std::string header_str = resp.str();
    lwip_send(sock, header_str.c_str(), header_str.size(), 0);
    headerSent = true;
}

// 2) writeChunk(...): send some body data
void Response::writeChunk(const char* data, size_t length)
{
    if (!headerSent) {
        // If the user forgot to call start(), do some fallback or error
        // but let's just do a default header or throw
        printf("Error: writeChunk called before start()\n");
        return;
    }
    #ifdef TRACE_ON
    printMemsize();
    #endif
    TRACE("Sending chunk of size: %zu\n", length);
    TRACE("Data: %.*s\n", (int)length, data);
    // Send the chunk data
    TRACE("Socket: %d\n", sock);
    ssize_t err = lwip_send(sock, data, length, 0);
    #ifdef TRACE_ON
    printMemsize();
    #endif
    if (err < 0) {
        printf("Error sending chunk: %zu\n", err);
        printf("Error: %s\n", strerror(errno));
    }
    else {
        TRACE("Chunk sent successfully\n");
    }
}

// 3) finish(): optional if you want a method to cleanly end. 
// With Content-Length, once we've sent that many bytes, we're done.
void Response::finish() {
    // In a typical content-length scenario, there's no "end chunk" to send.
    // If you used chunked encoding, you'd finalize with "0\r\n\r\n".
    // For now, we do nothing.
}

void Response::sendUnauthorized() {
    this->status(401)
         .set("Content-Type", "application/json")
         .send("{\"error\": \"Unauthorized\"}");
}

// res.setCookie("token", jwt, "HttpOnly; Path=/; Max-Age=3600"); // after login
// res.clearCookie("token", "Path=/"); // on logout
// res.setAuthorization(jwt);
// res.setCookie("token", jwt, "HttpOnly; Secure; SameSite=Strict; Path=/; Max-Age=3600");

Response& Response::setCookie(const std::string& name, const std::string& value, const std::string& options) {
    std::ostringstream cookie;
    cookie << name << "=" << value;
    if (!options.empty()) {
        cookie << "; " << options;
    }
    cookies.push_back(cookie.str());
    return *this;
}

Response& Response::clearCookie(const std::string& name, const std::string& options) {
    std::ostringstream cookie;
    cookie << name << "=; Max-Age=0";
    if (!options.empty()) {
        cookie << "; " << options;
    }
    cookies.push_back(cookie.str());
    return *this;
}

std::string Response::renderTemplate(const std::string& tpl, const std::map<std::string, std::string>& context) {
    std::string result = tpl;
    for (const auto& [key, value] : context) {
        std::string placeholder = "{{" + key + "}}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }
    return result;
}

// In HttpResponse.cpp

// Set the content type and return a reference for chaining.
Response& Response::setContentType(const std::string &ct) {
    headers["Content-Type"] = ct;
    return *this;
}

// Alias for setting headers (if you want it separate from set())
Response& Response::setHeader(const std::string &key, const std::string &value) {
    headers[key] = value;
    return *this;
}

// Alias for status (calls the status() method)
Response& Response::setStatus(int code) {
    return status(code);
}




