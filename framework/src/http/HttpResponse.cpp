/**
 * @file HttpResponse.cpp
 * @author Ian Archbell
 * @brief Implementation of the Response class for managing and sending HTTP responses.
 * 
 * Part of the PicoFramework HTTP server.
 * This module handles constructing HTTP responses, setting headers, and sending
 * the response back to the client. It also supports setting content types,
 * content lengths, and handling different response statuses.
 * 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #include "framework_config.h" // Must be included before DebugTrace.h to ensure framework_config.h is processed first
 #include "DebugTrace.h" 
 TRACE_INIT(HttpResponse)

 #include "HttpResponse.h"
 #include <sstream>
 #include <cstring>
 #include <lwip/sockets.h>
 #include "utility.h"
 
 // Immediately after lwip includes:
 #ifdef send
 #undef send
 #endif
 
 // ------------------------------------------------------------------------
 // Constructor
 // ------------------------------------------------------------------------
 
 /**
  * @brief Construct a new Response object.
  * @param sock Socket descriptor for the client connection.
  */
 Response::Response(int sock)
     : sock(sock), status_code(200), headerSent(false)
 {
 }
 
 // ------------------------------------------------------------------------
 // Status and Header Management
 // ------------------------------------------------------------------------
 
 /**
  * @copydoc Response::status()
  */
 Response& Response::status(int code)
 {
     status_code = code;
     return *this;
 }
 
 /**
  * @copydoc Response::setStatus()
  */
 Response& Response::setStatus(int code)
 {
     return status(code);
 }
 
 /**
  * @copydoc Response::set()
  */
 Response& Response::set(const std::string &field, const std::string &value)
 {
     headers[field] = value;
     return *this;
 }
 
 /**
  * @copydoc Response::setHeader()
  */
 Response& Response::setHeader(const std::string &key, const std::string &value)
 {
     headers[key] = value;
     return *this;
 }
 
 /**
  * @copydoc Response::setContentType()
  */
 Response& Response::setContentType(const std::string &ct)
 {
     headers["Content-Type"] = ct;
     return *this;
 }
 
 /**
  * @copydoc Response::setAuthorization()
  */
 Response& Response::setAuthorization(const std::string &jwtToken)
 {
     if (!jwtToken.empty())
     {
         headers["Authorization"] = "Bearer " + jwtToken;
     }
     return *this;
 }
 
 /**
  * @copydoc Response::getContentType()
  */
 std::string Response::getContentType() const
 {
     auto it = headers.find("Content-Type");
     return (it != headers.end()) ? it->second : "text/html";
 }
 
 /**
  * @copydoc Response::isHeaderSent()
  */
 bool Response::isHeaderSent() const
 {
     return headerSent;
 }
 
 /**
  * @copydoc Response::getSocket()
  */
 int Response::getSocket() const
 {
     return sock;
 }
 
 /**
  * @brief Return the standard HTTP status message for a code.
  * @param code HTTP status code.
  * @return Corresponding status message.
  */
 std::string Response::getStatusMessage(int code)
 {
     switch (code)
     {
         case 200: return "OK";
         case 401: return "Unauthorized";
         case 404: return "Not Found";
         case 500: return "Internal Server Error";
         default:  return "";
     }
 }
 
 // ------------------------------------------------------------------------
 // Cookie Support
 // ------------------------------------------------------------------------
 
 /**
  * @copydoc Response::setCookie()
  */
 Response& Response::setCookie(const std::string& name, const std::string& value, const std::string& options)
 {
     std::ostringstream cookie;
     cookie << name << "=" << value;
     if (!options.empty())
     {
         cookie << "; " << options;
     }
     cookies.push_back(cookie.str());
     return *this;
 }
 
 /**
  * @copydoc Response::clearCookie()
  */
 Response& Response::clearCookie(const std::string& name, const std::string& options)
 {
     std::ostringstream cookie;
     cookie << name << "=; Max-Age=0";
     if (!options.empty())
     {
         cookie << "; " << options;
     }
     cookies.push_back(cookie.str());
     return *this;
 }
 
 // ------------------------------------------------------------------------
 // Body and Streaming
 // ------------------------------------------------------------------------
 
 /**
  * @copydoc Response::send()
  */
 void Response::send(const std::string &body)
 {
     if (!headerSent)
     {
         if (headers.find("Content-Length") == headers.end())
         {
             headers["Content-Length"] = std::to_string(body.size());
         }
         if (headers.find("Content-Type") == headers.end())
         {
             headers["Content-Type"] = "text/html";
         }
 
         std::ostringstream resp;
         resp << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
         for (auto &h : headers)
         {
             resp << h.first << ": " << h.second << "\r\n";
         }
         for (const auto& cookie : cookies)
         {
             resp << "Set-Cookie: " << cookie << "\r\n";
         }
         resp << "\r\n";
 
         std::string header_str = resp.str();
         lwip_send(sock, header_str.c_str(), header_str.size(), 0);
         headerSent = true;
     }
 
     lwip_send(sock, body.data(), body.size(), 0);
 }
 
 /**
  * @copydoc Response::sendHeaders()
  */
 void Response::sendHeaders()
 {
     if (!headerSent)
     {
         std::ostringstream resp;
         resp << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
 
         for (auto &h : headers)
         {
             resp << h.first << ": " << h.second << "\r\n";
         }
         for (const auto& cookie : cookies)
         {
             resp << "Set-Cookie: " << cookie << "\r\n";
         }
         if (headers.find("Connection") == headers.end())
         {
             resp << "Connection: close\r\n";
         }
         resp << "\r\n";
 
         std::string header_str = resp.str();
         lwip_send(sock, header_str.c_str(), header_str.size(), 0);
         headerSent = true;
     }
 }
 
 /**
  * @copydoc Response::start()
  */
 void Response::start(int code, size_t contentLength, const std::string &contentType)
 {
     status_code = code;
     headers["Content-Length"] = std::to_string(contentLength);
     headers["Content-Type"] = contentType;
 
     std::ostringstream resp;
     resp << "HTTP/1.1 " << status_code << " " << getStatusMessage(status_code) << "\r\n";
     for (auto &h : headers)
     {
         resp << h.first << ": " << h.second << "\r\n";
     }
     for (const auto& cookie : cookies)
     {
         resp << "Set-Cookie: " << cookie << "\r\n";
     }
     resp << "\r\n";
 
     std::string header_str = resp.str();
     lwip_send(sock, header_str.c_str(), header_str.size(), 0);
     headerSent = true;
 }
 
 /**
  * @copydoc Response::writeChunk()
  */
 void Response::writeChunk(const char* data, size_t length)
 {
     if (!headerSent)
     {
         printf("Error: writeChunk called before start()\n");
         return;
     }
 
     ssize_t err = lwip_send(sock, data, length, 0);
     if (err < 0)
     {
         printf("Error sending chunk: %zu\n", err);
         printf("Error: %s\n", strerror(errno));
     }
 }
 
 /**
  * @copydoc Response::finish()
  */
 void Response::finish()
 {
     // Placeholder for future expansion (e.g., chunked transfer end).
 }
 
 // ------------------------------------------------------------------------
 // Helpers
 // ------------------------------------------------------------------------
 
 /**
  * @copydoc Response::sendUnauthorized()
  */
 void Response::sendUnauthorized()
 {
     this->status(401)
         .set("Content-Type", "application/json")
         .send("{\"error\": \"Unauthorized\"}");
 }
 
 /**
  * @copydoc Response::renderTemplate()
  */
 std::string Response::renderTemplate(const std::string& tpl, const std::map<std::string, std::string>& context)
 {
     std::string result = tpl;
     for (const auto& [key, value] : context)
     {
         std::string placeholder = "{{" + key + "}}";
         size_t pos = 0;
         while ((pos = result.find(placeholder, pos)) != std::string::npos)
         {
             result.replace(pos, placeholder.length(), value);
             pos += value.length();
         }
     }
     return result;
 }
 