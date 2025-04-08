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
