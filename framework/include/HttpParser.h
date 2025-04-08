#pragma once
#include <string>
#include <map>
#include "TcpConnectionSocket.h"

class HttpParser
{
public:
    /**
     * @brief Receive raw headers from the socket until \r\n\r\n
     */
    static bool receiveHeaders(TcpConnectionSocket &socket, std::string &outHeaders);

    /**
     * @brief Receive the HTTP body from socket, using Content-Length or connection close
     */
    static bool receiveBody(TcpConnectionSocket &socket,
                            const std::map<std::string, std::string> &headers,
                            std::string &outBody);
    static int parseStatusCode(const std::string &statusLine);
    static std::map<std::string, std::string> parseHeaders(const std::string &rawHeaders);
};
