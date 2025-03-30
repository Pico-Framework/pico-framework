#pragma once

#include <unordered_map>
#include <string>
#include <sstream>

std::unordered_map<std::string, std::string> parseQuery(const std::string& query);
std::unordered_map<std::string, std::string> parseUrlEncoded(const std::string& body);

inline std::string trim(const std::string& s);

// Function to decode URL-encoded strings
// This function decodes percent-encoded characters (e.g., %20 for space) and replaces '+' with space
std::string decodeURIComponent(const std::string& str);

std::string getClientIpFromSocket(int sock);



