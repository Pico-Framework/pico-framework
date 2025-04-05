#pragma once
#include <string>
#include <unordered_map>

class HttpParser {
public:
    static int parseStatusCode(const std::string& statusLine);
    static std::unordered_map<std::string, std::string> parseHeaders(const std::string& rawHeaders);
};
