/**
 * @file HttpParser.cpp
 * @author Ian Archbell
 * @brief HTTP parser for status codes, headers, and body handling.
 * Part of the PicoFramework HTTP server.
 * This module provides methods to parse HTTP status lines, headers, and
 * handle HTTP body content, including chunked transfer encoding and
 * content-length handling.
 * It is designed for use in embedded systems with FreeRTOS and lwIP.
 * @version 0.1
 * @date 2025-03-26
 * @license MIT License
 * @copyright (c) 2025, Ian Archbell
 *
 */

#pragma once
#include <string>
#include <map>
#include <functional>
#include "network/Tcp.h"

class HttpParser
{
public:
    /**
     * @brief Receive raw headers from the socket until \r\n\r\n
     * @param socket The socket to receive from
     * @param outHeaders The received headers
     * @param leftoverBody The leftover body after headers
     * @return true on success, false on failure
     */

    static std::pair<std::string, std::string> receiveHeaderAndLeftover(Tcp &socket);

    /**
     * @brief Receive the HTTP body from socket, using Content-Length or connection close
     */
    static bool receiveBody(Tcp &socket,
        const std::map<std::string, std::string> &headers,
        const std::string &leftoverBody,
        std::string &outBody,
        size_t maxLength,
        bool *wasTruncated);

    static bool receiveChunkedBodyToFile(
        Tcp& socket,
        const std::string& leftover,
        std::function<bool(const char*, size_t)> writeFn,
        size_t maxLength,
        bool* wasTruncated);
        
    static bool receiveFixedLengthBodyToFile(
        Tcp& socket,
        const std::map<std::string, std::string>& headers,
        const std::string& leftover,
        std::function<bool(const char*, size_t)> writeFn,
        size_t maxLength,
        bool* wasTruncated);
                            
    static int parseStatusCode(const std::string &statusLine);
    static std::map<std::string, std::string> parseHeaders(const std::string &rawHeaders);

    static bool isChunkedEncoding(const std::map<std::string, std::string> &headers);
    static bool receiveChunkedBodyToString(Tcp &socket, const std::string &leftover, std::string &out, size_t maxLength, bool *wasTruncated);
    static bool receiveFixedLengthBodyToString(Tcp &socket, const std::map<std::string, std::string> &headers, const std::string &leftover, std::string &out, size_t maxLength, bool *wasTruncated);
    static bool receiveUnknownLengthBodyToString(Tcp &socket, const std::string &leftover, std::string &out, size_t maxLength, bool *wasTruncated);
};
