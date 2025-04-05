#include "TcpConnectionSocket.h"
#include <lwip/sockets.h>  // or <sys/socket.h> on host
#include <unistd.h>

TcpConnectionSocket::TcpConnectionSocket(int sockfd)
    : sockfd(sockfd) {}

TcpConnectionSocket::~TcpConnectionSocket() {
    if (sockfd >= 0) {
        close();
    }
}

TcpConnectionSocket::TcpConnectionSocket(TcpConnectionSocket&& other) noexcept
    : sockfd(other.sockfd) {
    other.sockfd = -1;
}

TcpConnectionSocket& TcpConnectionSocket::operator=(TcpConnectionSocket&& other) noexcept {
    if (this != &other) {
        close();
        sockfd = other.sockfd;
        other.sockfd = -1;
    }
    return *this;
}

#include <lwip/sockets.h>  // or <sys/socket.h>
#include <cstring>

TcpConnectionSocket::TcpConnectionSocket() : sockfd(-1) {}

bool TcpConnectionSocket::bindAndListen(int port) {
    sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (lwip_bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0) return false;
    return lwip_listen(sockfd, 5) == 0;
}

TcpConnectionSocket TcpConnectionSocket::accept() {
    sockaddr_in clientAddr{};
    socklen_t addrLen = sizeof(clientAddr);
    int clientSock = lwip_accept(sockfd, (sockaddr*)&clientAddr, &addrLen);
    return TcpConnectionSocket(clientSock);
}


bool TcpConnectionSocket::isValid() const {
    return sockfd >= 0;
}

int TcpConnectionSocket::getSocketFd() const {
    return sockfd;
}

int TcpConnectionSocket::recv(char* buffer, size_t size) {
    return lwip_recv(sockfd, buffer, size, 0);
}

int TcpConnectionSocket::send(const char* buffer, size_t size) {
    return lwip_send(sockfd, buffer, size, 0);
}

int TcpConnectionSocket::close() {
    int result = 0;
    if (sockfd >= 0) {
        result = lwip_close(sockfd);
        sockfd = -1;
    }
    return result;
}
