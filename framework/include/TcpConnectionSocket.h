#pragma once

#include <string>
#include <vector>

class TcpConnectionSocket {
public:
    explicit TcpConnectionSocket(int sockfd);
    ~TcpConnectionSocket();

    // Disable copy
    TcpConnectionSocket(const TcpConnectionSocket&) = delete;
    TcpConnectionSocket& operator=(const TcpConnectionSocket&) = delete;

    // Allow move
    TcpConnectionSocket(TcpConnectionSocket&& other) noexcept;
    TcpConnectionSocket& operator=(TcpConnectionSocket&& other) noexcept;

    bool isValid() const;
    int getSocketFd() const;

    int recv(char* buffer, size_t size);
    int send(const char* buffer, size_t size);
    int close();

    TcpConnectionSocket(); // Default constructor for listener
    bool bindAndListen(int port);
    TcpConnectionSocket accept();


private:
    int sockfd;
};
