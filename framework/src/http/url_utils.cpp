#include "url_utils.h"
#include <sstream>
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/memp.h"
#include <string>
#include <algorithm>
#include <cctype>

// Helper function to decode a URL-encoded string.
std::string urlDecode(const std::string &src) {
    std::string ret;
    for (size_t i = 0; i < src.length(); ++i) {
        if (src[i] == '%') {
            if (i + 2 < src.length()) {
                std::istringstream iss(src.substr(i + 1, 2));
                int hexVal;
                if (iss >> std::hex >> hexVal) {
                    ret += static_cast<char>(hexVal);
                    i += 2;
                } else {
                    ret += '%';
                }
            } else {
                ret += '%';
            }
        } else if (src[i] == '+') {
            ret += ' ';
        } else {
            ret += src[i];
        }
    }
    return ret;
}

std::unordered_map<std::string, std::string> parseUrlEncoded(const std::string &data) {
    std::unordered_map<std::string, std::string> params;
    std::istringstream stream(data);
    std::string pair;
    while (std::getline(stream, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            // Decode both key and value
            params[urlDecode(key)] = urlDecode(value);
        }
    }
    return params;
}

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

std::string getMimeType(const std::string& filePath) {

    static const std::unordered_map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".txt", "text/plain"},
        {".xml", "application/xml"},
        {".pdf", "application/pdf"},
        {".zip", "application/zip"},
        {".mp4", "video/mp4"},
        {".mp3", "audio/mpeg"},
        {".wav", "audio/wav"},
        {".csv", "text/csv"}
    };

    size_t extPos = filePath.find_last_of(".");
    if (extPos != std::string::npos) {
        std::string ext = filePath.substr(extPos); // Extract extension

        auto it = mimeTypes.find(ext);
        if (it != mimeTypes.end()) {
            return it->second;  // Return corresponding MIME type
        }
    }

    return "application/octet-stream";  // Default MIME type
}
