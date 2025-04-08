#pragma once
#include <string>
#include <map>

class HttpParser {
public:
    static int parseStatusCode(const std::string& statusLine);
    static std::map<std::string, std::string> parseHeaders(const std::string& rawHeaders);
};
