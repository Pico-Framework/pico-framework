/**
 * @file HttpParser.cpp
 * @author Ian Archbell
 * @brief HTTP parser for status codes, headers, and body handling.
 * * Part of the PicoFramework HTTP server.
 * This module provides methods to parse HTTP status lines, headers, and
 * handle HTTP body content, including chunked transfer encoding and
 * content-length handling.
 * It is designed for use in embedded systems with FreeRTOS and lwIP.
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 *
 */

#include "http/HttpParser.h"
#include <sstream>
#include <algorithm>
#include <map>

#include "http/ChunkedDecoder.h"
#include "utility/utility.h"
#include "framework_config.h"
#include "DebugTrace.h"
TRACE_INIT(HttpParser)
/**
 * @brief Parses the HTTP status line to extract the status code.
 * @param statusLine The HTTP status line (e.g., "HTTP/1.1 200 OK").
 * @return The HTTP status code as an integer (e.g., 200).
 */
int HttpParser::parseStatusCode(const std::string &statusLine)
{
    std::istringstream stream(statusLine);
    std::string httpVersion;
    int code = 0;
    stream >> httpVersion >> code;
    return code;
}
/**
 * @brief Parses the HTTP headers from a raw header string.
 * @param rawHeaders The raw HTTP headers as a string.
 * @return A map of header names to their values.
 */
std::map<std::string, std::string> HttpParser::parseHeaders(const std::string &rawHeaders)
{
    std::map<std::string, std::string> headers;
    std::istringstream stream(rawHeaders);
    std::string line;

    while (std::getline(stream, line))
    {
        // Stop on blank line (end of headers)
        if (line == "\r" || line.empty())
        {
            break;
        }

        auto colon = line.find(':');
        if (colon == std::string::npos || colon + 1 >= line.size())
        {
            continue;
        }

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        // Remove CR and quotes
        key.erase(std::remove(key.begin(), key.end(), '\r'), key.end());
        value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
        key.erase(std::remove(key.begin(), key.end(), '"'), key.end());
        value.erase(std::remove(value.begin(), value.end(), '"'), value.end());

        // Lowercase key
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

        // Trim whitespace from value
        size_t start = value.find_first_not_of(" \t");
        size_t end = value.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos)
        {
            value = value.substr(start, end - start + 1);
        }
        else
        {
            value = "";
        }

        headers[key] = value;
    }

    return headers;
}

/**
 * @brief Receives HTTP headers and any leftover body data from a TCP socket.
 * @param socket The TCP socket to read from.
 * @return A pair containing the full header text and any leftover body data.
 * If an error occurs, returns an empty header and leftover.
 * This function reads data from the socket until it finds the end of the headers
 * (indicated by a double CRLF "\r\n\r\n"). It accumulates data in a buffer
 * and returns the complete header text along with any leftover body data that
 * may have been received after the headers.
 * It handles cases where the header may be split across multiple TCP packets.
 * If the end of the headers is not found, it continues to read from the socket
 * until it is found or an error occurs.
 * @note This function is blocking and will wait until the header is fully received.
 * It is designed for use in an embedded system with FreeRTOS and lwIP.
 */
std::pair<std::string, std::string> HttpParser::receiveHeaderAndLeftover(Tcp &socket)
{
    std::string buffer;
    char temp[1460];

    while (true)
    {
        int n = socket.recv(temp, sizeof(temp));
        if (n <= 0)
        {
            return {"", ""}; // signal error (empty header)
        }

        buffer.append(temp, n);
        std::size_t headerEnd = buffer.find("\r\n\r\n");

        if (headerEnd != std::string::npos)
        {
            std::string headerText = buffer.substr(0, headerEnd + 4);
            std::string leftover = buffer.substr(headerEnd + 4);
            return {headerText, leftover};
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/**
 * @brief Receives the HTTP body from a TCP socket based on headers.
 * @param socket The TCP socket to read from.
 * @param headers The parsed HTTP headers.
 * @param leftoverBody Any body data that was received after the headers.   
 * @param outBody The output string to store the received body.
 * @param maxLength The maximum length of the body to receive.
 * @param wasTruncated Pointer to a boolean that will be set to true if the body was truncated.
 * @return True if the body was successfully received, false on error or EOF.
 * This function handles different content transfer methods:
 * - Chunked transfer encoding
 * - Content-Length specified in headers
 * - Unknown length (just read until EOF)
 */
bool HttpParser::receiveBody(Tcp &socket,
                             const std::map<std::string, std::string> &headers,
                             const std::string &leftoverBody,
                             std::string &outBody,
                             size_t maxLength,
                             bool *wasTruncated)
{
    if (wasTruncated)
        *wasTruncated = false;

    if (isChunkedEncoding(headers))
    {
        return receiveChunkedBodyToString(socket, leftoverBody, outBody, maxLength, wasTruncated);
    }

    if (headers.count("content-length"))
    {
        return receiveFixedLengthBodyToString(socket, headers, leftoverBody, outBody, maxLength, wasTruncated);
    }

    return receiveUnknownLengthBodyToString(socket, leftoverBody, outBody, maxLength, wasTruncated);
}

bool HttpParser::isChunkedEncoding(const std::map<std::string, std::string>& headers)
{
    auto it = headers.find("transfer-encoding");
    return (it != headers.end() && toLower(it->second) == "chunked");
}

bool HttpParser::receiveChunkedBodyToString(Tcp& socket, const std::string& leftover, std::string& outBody, size_t maxLength, bool* wasTruncated)
{
    ChunkedDecoder decoder;
    decoder.feed(leftover, maxLength);

    char temp[1460];
    while (!decoder.isComplete()) {
        int n = socket.recv(temp, sizeof(temp));
        if (n <= 0) {
            printf("Chunked: recv() failed or EOF");
            return false;
        }
        decoder.feed(std::string(temp, n), maxLength);
        if (decoder.wasTruncated()) {
            outBody = decoder.getDecoded();
            if (wasTruncated) *wasTruncated = true;
            return true;
        }
    }

    outBody = decoder.getDecoded();
    return true;
}

bool HttpParser::receiveFixedLengthBodyToString(Tcp& socket, const std::map<std::string, std::string>& headers, const std::string& leftover, std::string& outBody, size_t maxLength, bool* wasTruncated)
{
    int contentLength = std::stoi(headers.at("content-length"));
    outBody = leftover;

    if ((int)outBody.size() >= contentLength) {
        outBody = outBody.substr(0, std::min((size_t)contentLength, maxLength));
        if ((size_t)contentLength > maxLength && wasTruncated) *wasTruncated = true;
        return true;
    }

    int attempts = 0;
    int idleCycles = 0;
    while ((int)outBody.size() < contentLength && attempts++ < 2000) {
        char buffer[1460];
        int toRead = std::min<int>(sizeof(buffer), contentLength - (int)outBody.size());
        int n = socket.recv(buffer, toRead);

        if (n <= 0) {
            idleCycles++;
            vTaskDelay(pdMS_TO_TICKS(10));
            if (idleCycles > 20) return false;
            continue;
        }

        idleCycles = 0;
        if (outBody.size() + n > maxLength) {
            outBody.append(buffer, maxLength - outBody.size());
            if (wasTruncated) *wasTruncated = true;
            return true;
        }

        outBody.append(buffer, n);
    }

    return ((int)outBody.size() == contentLength);
}

bool HttpParser::receiveUnknownLengthBodyToString(Tcp& socket, const std::string& leftover, std::string& outBody, size_t maxLength, bool* wasTruncated)
{
    outBody = leftover;
    char buffer[1460];
    while (true) {
        int n = socket.recv(buffer, sizeof(buffer));
        if (n <= 0) break;

        if (outBody.size() + n > maxLength) {
            outBody.append(buffer, maxLength - outBody.size());
            if (wasTruncated) *wasTruncated = true;
            return true;
        }

        outBody.append(buffer, n);
    }

    return !outBody.empty();
}


bool HttpParser::receiveChunkedBodyToFile(
    Tcp& socket,
    const std::string& leftover,
    std::function<bool(const char*, size_t)> writeFn,
    size_t maxLength,
    bool* wasTruncated)
{
    ChunkedDecoder decoder;
    decoder.feedToFile(leftover, writeFn, maxLength);

    char buffer[1460];
    while (!decoder.isComplete()) {
        int n = socket.recv(buffer, sizeof(buffer));
        if (n <= 0) {
            printf("Chunked: recv() failed or EOF\n");
            return false;
        }

        if (!decoder.feedToFile(std::string(buffer, n), writeFn, maxLength)) {
            printf("Chunked: writeFn failed\n");
            return false;
        }

        if (decoder.wasTruncated()) {
            if (wasTruncated) *wasTruncated = true;
            return true;
        }
    }

    if (wasTruncated) *wasTruncated = decoder.wasTruncated();
    return true;
}

bool HttpParser::receiveFixedLengthBodyToFile(
    Tcp& socket,
    const std::map<std::string, std::string>& headers,
    const std::string& leftover,
    std::function<bool(const char*, size_t)> writeFn,
    size_t maxLength,
    bool* wasTruncated)
{
    if (wasTruncated) *wasTruncated = false;

    auto it = headers.find("content-length");
    if (it == headers.end()) return false;

    int contentLength = std::stoi(it->second);
    size_t totalWritten = 0;

    // Write leftover first
    if (!leftover.empty()) {
        size_t toWrite = std::min(leftover.size(), maxLength);
        if (!writeFn(leftover.data(), toWrite)) return false;
        totalWritten += toWrite;

        if (leftover.size() > maxLength) {
            if (wasTruncated) *wasTruncated = true;
            return true;
        }
    }

    char buffer[1460];
    int idleCycles = 0;
    while (totalWritten < (size_t)contentLength) {
        size_t remaining = contentLength - totalWritten;
        int toRead = std::min((int)sizeof(buffer), (int)remaining);
        int n = socket.recv(buffer, toRead);

        if (n <= 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
            if (++idleCycles > 20) return false;
            continue;
        }

        idleCycles = 0;

        size_t available = maxLength - totalWritten;
        size_t toWrite = std::min((size_t)n, available);
        if (!writeFn(buffer, toWrite)) return false;
        totalWritten += toWrite;

        if (toWrite < (size_t)n) {
            if (wasTruncated) *wasTruncated = true;
            return true;
        }
    }

    return true;
}



