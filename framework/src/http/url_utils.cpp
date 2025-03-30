#include "url_utils.h"
#include <sstream>
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/memp.h"
#include <string>
#include <algorithm>
#include <cctype>

inline std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::unordered_map<std::string, std::string> parseQuery(const std::string& query) {
    std::unordered_map<std::string, std::string> params;
    std::istringstream stream(query);
    std::string pair;
    while (std::getline(stream, pair, '&')) {
        size_t equal = pair.find('=');
        if (equal != std::string::npos) {
            params[pair.substr(0, equal)] = pair.substr(equal + 1);
        }
    }
    return params;
}

std::unordered_map<std::string, std::string> parseUrlEncoded(const std::string& body) {
    return parseQuery(body);
}

// Function to decode URL-encoded strings
// This function decodes percent-encoded characters (e.g., %20 for space) and replaces '+' with space
std::string decodeURIComponent(const std::string& str) {
    std::string decoded;
    char hexBuffer[3] = {0};
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            hexBuffer[0] = str[i + 1];
            hexBuffer[1] = str[i + 2];
            decoded += static_cast<char>(std::strtol(hexBuffer, nullptr, 16));
            i += 2;
        } else if (str[i] == '+') {
            decoded += ' ';
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}

// std::unordered_map<std::string, std::string> parseUrlEncoded(const std::string& input) {
//     std::unordered_map<std::string, std::string> params;
//     std::istringstream stream(input);
//     std::string keyValue;

//     while (std::getline(stream, keyValue, '&')) {
//         size_t eqPos = keyValue.find('=');
//         if (eqPos != std::string::npos) {
//             std::string key = decodeURIComponent(keyValue.substr(0, eqPos));
//             std::string value = decodeURIComponent(keyValue.substr(eqPos + 1));
//             params[key] = value;
//         }
//     }
//     return params;
// }

std::string getClientIpFromSocket(int sock) {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);

    if (lwip_getpeername(sock, (sockaddr*)&addr, &len) == 0) {
        ip_addr_t ip;
        ip.addr = addr.sin_addr.s_addr;
        return std::string(ipaddr_ntoa(&ip));  // Make a copy of the static buffer
    }
    return "0.0.0.0";  // Fallback if getpeername fails
}


