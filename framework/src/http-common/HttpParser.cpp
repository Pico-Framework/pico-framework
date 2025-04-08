#include "HttpParser.h"
#include <sstream>
#include <algorithm>
#include <map>

int HttpParser::parseStatusCode(const std::string& statusLine) {
    std::istringstream stream(statusLine);
    std::string httpVersion;
    int code = 0;
    stream >> httpVersion >> code;
    return code;
}

std::map<std::string, std::string> HttpParser::parseHeaders(const std::string& rawHeaders) {
    std::map<std::string, std::string> headers;
    std::istringstream stream(rawHeaders);
    std::string line;

    while (std::getline(stream, line)) {
        // Stop on blank line (end of headers)
        if (line == "\r" || line.empty()) {
            break;
        }

        auto colon = line.find(':');
        if (colon == std::string::npos || colon + 1 >= line.size()) {
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
        if (start != std::string::npos && end != std::string::npos) {
            value = value.substr(start, end - start + 1);
        } else {
            value = "";
        }

        headers[key] = value;
    }

    return headers;
}

bool HttpParser::receiveHeaders(TcpConnectionSocket& socket, std::string& outHeaders) {
    char buffer[1024];
    std::string data;
    while (true) {
        int n = socket.recv(buffer, sizeof(buffer));
        if (n <= 0) return false;
        data.append(buffer, n);
        auto end = data.find("\r\n\r\n");
        if (end != std::string::npos) {
            outHeaders = data.substr(0, end + 4);
            return true;
        }
    }
}

bool HttpParser::receiveBody(TcpConnectionSocket& socket,
                             const std::map<std::string, std::string>& headers,
                             std::string& outBody) {
    auto it = headers.find("Content-Length");
    if (it != headers.end()) {
        int contentLength = std::stoi(it->second);
        outBody.clear();
        while ((int)outBody.size() < contentLength) {
            char buffer[1024];
            int n = socket.recv(buffer, std::min<int>((int)sizeof(buffer), contentLength - (int)outBody.size()));

            if (n <= 0) return false;
            outBody.append(buffer, n);
        }
        return true;
    }

    // Fallback: read until socket closes (not ideal, but valid for no content-length)
    char buffer[1024];
    while (true) {
        int n = socket.recv(buffer, sizeof(buffer));
        if (n <= 0) break;
        outBody.append(buffer, n);
    }
    return !outBody.empty();
}

