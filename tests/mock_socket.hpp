#ifndef MOCK_SOCKET_HPP
#define MOCK_SOCKET_HPP

#include <queue>
#include <string>
#include <vector>

class MockSocket {
public:
    MockSocket() = default;

    void setIncomingData(const std::vector<std::string>& chunks) {
        for (const auto& chunk : chunks) {
            receivedData.push(chunk);
        }
    }

    int recv(char* buffer, int size) {
        if (receivedData.empty()) return 0; // Simulate EOF
        std::string data = receivedData.front();
        receivedData.pop();

        int bytesToCopy = std::min(size, (int)data.size());
        std::memcpy(buffer, data.c_str(), bytesToCopy);
        return bytesToCopy;
    }

    void close() {}

private:
    std::queue<std::string> receivedData;
};

#endif // MOCK_SOCKET_HPP
