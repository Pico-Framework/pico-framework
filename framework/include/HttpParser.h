#pragma once
#include <string>
#include <map>
#include "TcpConnectionSocket.h"

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

    static std::pair<std::string, std::string> receiveHeaderAndLeftover(TcpConnectionSocket &socket);

    /**
     * @brief Receive the HTTP body from socket, using Content-Length or connection close
     */
    static bool receiveBody(TcpConnectionSocket& socket,
        const std::map<std::string, std::string>& headers,
        const std::string& leftoverBody,
        std::string& outBody);
    static int parseStatusCode(const std::string &statusLine);
    static std::map<std::string, std::string> parseHeaders(const std::string &rawHeaders);
};
