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
        // Build the headers based on internal member variables
        std::ostringstream resp;
        resp << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
        resp << "Content-Type: " << content_type << "\r\n";

        // Add content length if available
        if (!content_length.empty()) {
            resp << "Content-Length: " << content_length << "\r\n";
        }

        // Optionally add any other headers, e.g., Connection, Cache-Control
        resp << "Connection: close\r\n";  // Or 'keep-alive' depending on your use case

        resp << "\r\n";  // Blank line to end headers

        // Convert to string and send
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
        printf("Error sending chunk: %d\n", err);
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



