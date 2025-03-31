/**
 * @file url_utils.h
 * @brief Utility functions for URL decoding, form parsing, MIME type lookup, and IP address extraction.
 * 
 * These functions support core HTTP functionality including decoding URL-encoded strings, 
 * parsing application/x-www-form-urlencoded bodies, detecting client IP addresses, and 
 * determining appropriate MIME types for static file serving.
 * 
 * @version 0.1
 * @date 2025-03-31
 * @author ...
 * @copyright MIT
 */

 #pragma once

 #include <string>
 #include <unordered_map>
 #include <sstream>
 
 /**
  * @brief Trim whitespace from the beginning and end of a string.
  * 
  * @param s The input string.
  * @return A new string with leading/trailing whitespace removed.
  */
 inline std::string trim(const std::string& s) {
     size_t start = s.find_first_not_of(" \t\r\n");
     size_t end = s.find_last_not_of(" \t\r\n");
     return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
 }
 
 /**
  * @brief Decode a URL-encoded string (e.g., "a%20b+c" -> "a b c").
  * 
  * Converts percent-encoded characters and replaces '+' with space.
  * 
  * @param src The URL-encoded input string.
  * @return The decoded string.
  */
 std::string urlDecode(const std::string &src);
 
 /**
  * @brief Parse a URL-encoded key-value string into a map.
  * 
  * Converts data in the format "key1=value1&key2=value2" into a map.
  * Both keys and values are URL-decoded.
  * 
  * @param data URL-encoded form data.
  * @return A map of key-value pairs.
  */
 std::unordered_map<std::string, std::string> parseUrlEncoded(const std::string &data);
 
 /**
  * @brief Get the client IP address from a socket.
  * 
  * Uses lwIP's `lwip_getpeername()` to extract the IP address of the remote peer.
  * 
  * @param sock The socket file descriptor.
  * @return The client IP address as a string. Returns "0.0.0.0" on failure.
  */
 std::string getClientIpFromSocket(int sock);
 
 /**
  * @brief Get the MIME type based on a file path's extension.
  * 
  * Used when serving static files to determine the appropriate `Content-Type`.
  * 
  * @param filePath The file path (e.g., "/index.html").
  * @return A MIME type string (e.g., "text/html").
  */
 std::string getMimeType(const std::string& filePath);
 