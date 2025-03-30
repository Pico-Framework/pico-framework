#pragma once

#include <string>
#include <unordered_map>
#include <sstream>

// Trim whitespace from the beginning and end of a string.
inline std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    size_t end = s.find_last_not_of(" \t\r\n");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Function to decode URL-encoded strings.
std::string urlDecode(const std::string &src);

// Parse URL-encoded data into a key-value map.
std::unordered_map<std::string, std::string> parseUrlEncoded(const std::string &data);

// Get client IP address from socket.
std::string getClientIpFromSocket(int sock);

std::string getMimeType(const std::string& filePath);
 