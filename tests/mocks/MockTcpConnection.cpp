#pragma once

#include "TcpConnection.h"

class MockTcpConnection : public TcpConnection {
public:
    int receive(void* buf, size_t len) override { return 0; }
    int send(const void* buf, size_t len) override { return 0; }
    void close() override {}
    std::string getClientIp() const override { return "127.0.0.1"; }
};
