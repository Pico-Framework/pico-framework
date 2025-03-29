#pragma once

#include <string>

class TcpConnection {
public:
    virtual int receive(void* buf, size_t len) = 0;
    virtual int send(const void* buf, size_t len) = 0;
    virtual void close() = 0;
    virtual std::string getClientIp() const = 0;
    virtual ~TcpConnection() = default;
};
