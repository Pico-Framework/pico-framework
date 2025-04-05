#include "TcpConnectionSocket.h"
#include <lwip/sockets.h> // or <sys/socket.h> on host
#include <unistd.h>
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip_dns_resolver.h"
#include "lwip/ip4_addr.h"
#include <lwip/sockets.h>
#include <lwip/netdb.h>

#include "framework_config.h"
#include "DebugTrace.h"
TRACE_INIT(TcpConnectionSocket)

TcpConnectionSocket::TcpConnectionSocket(int sockfd)
    : sockfd(sockfd) {}

TcpConnectionSocket::~TcpConnectionSocket()
{
    if (sockfd >= 0)
    {
        close();
    }
}

TcpConnectionSocket::TcpConnectionSocket(TcpConnectionSocket &&other) noexcept
    : sockfd(other.sockfd)
{
    other.sockfd = -1;
}

TcpConnectionSocket &TcpConnectionSocket::operator=(TcpConnectionSocket &&other) noexcept
{
    if (this != &other)
    {
        close();
        sockfd = other.sockfd;
        other.sockfd = -1;
    }
    return *this;
}

#include <lwip/sockets.h> // or <sys/socket.h>
#include <cstring>

TcpConnectionSocket::TcpConnectionSocket() : sockfd(-1) {}

bool TcpConnectionSocket::bindAndListen(int port)
{
    sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (lwip_bind(sockfd, (sockaddr *)&addr, sizeof(addr)) < 0)
        return false;
    return lwip_listen(sockfd, 5) == 0;
}

TcpConnectionSocket TcpConnectionSocket::accept()
{
    sockaddr_in clientAddr{};
    socklen_t addrLen = sizeof(clientAddr);
    int clientSock = lwip_accept(sockfd, (sockaddr *)&clientAddr, &addrLen);
    return TcpConnectionSocket(clientSock);
}

int TcpConnectionSocket::getSocketFd() const
{
    return sockfd;
}

int TcpConnectionSocket::recv(char *buffer, size_t size)
{
    return lwip_recv(sockfd, buffer, size, 0);
}

int TcpConnectionSocket::send(const char *buffer, size_t size)
{
    return lwip_send(sockfd, buffer, size, 0);
}

int TcpConnectionSocket::close()
{
    int result = 0;
    if (sockfd >= 0)
    {
        result = lwip_close(sockfd);
        sockfd = -1;
    }
    return result;
}

bool TcpConnectionSocket::connect(const char* host, int port) {
    TRACE("TcpConnectionSocket", "Starting connect to %s:%d", host, port);

    ip_addr_t ip;
    TRACE("TcpConnectionSocket", "Resolving hostname: %s", host);
    if (!resolveHostnameBlocking(host, &ip)) {
        TRACE("TcpConnectionSocket", "DNS lookup failed for: %s", host);
        return false;
    }

    TRACE("TcpConnectionSocket", "DNS resolved to: %s", ipaddr_ntoa(&ip));

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = lwip_htons(port);
    addr.sin_addr.s_addr = ip.addr;

    TRACE("TcpConnectionSocket", "Creating socket...");
    sockfd = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        TRACE("TcpConnectionSocket", "Socket creation failed");
        return false;
    }

    TRACE("TcpConnectionSocket", "Connecting to %s:%d...", host, port);
    if (lwip_connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        TRACE("TcpConnectionSocket", "Connection to %s:%d failed", host, port);
        lwip_close(sockfd);
        sockfd = -1;
        return false;
    }

    TRACE("TcpConnectionSocket", "Connection successful to %s:%d", host, port);
    return true;
}
