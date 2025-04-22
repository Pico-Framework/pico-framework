/**
 * @file HttpResponse.h
 * @author Ian Archbell
 * @brief HTTP HttpResponse class for managing status, headers, body, and streaming support.
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 *
 * This class provides methods for setting HTTP response metadata,
 * sending body content, streaming chunks, and working with cookies.
 * Designed for use in embedded HTTP servers.
 */

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include "nlohmann/json.hpp"
#include "Tcp.h"

// Forward declaration of HttpRequest class
class HttpRequest;

/**
 * @brief Represents an HTTP response object.
 *
 * Used by the server to construct and send headers, content, and cookies
 * in response to an incoming HTTP request.
 */
class HttpResponse
{

public:
    HttpResponse() = default;

    /**
     * @brief Construct a new HttpResponse object with a socket.
     * @param sock The socket file descriptor.
     */
    HttpResponse(Tcp *tcp);

    Tcp *getTcp() const { return tcp; }

    // ------------------------------------------------------------------------
    // Status and Header Management
    // ------------------------------------------------------------------------

    /**
     * @brief Set the HTTP status code.
     * @param code Status code (e.g., 200, 404).
     * @return Reference to this HttpResponse object.
     */
    HttpResponse &status(int code);

    /**
     * @brief Alias for status().
     */
    HttpResponse &setStatus(int code);

    /**
     * @brief Set the body of the response (string).
     * @param body Response body content
     */
    HttpResponse &setBody(const std::string &body);

    /**
     * @brief Set the Content-Type header.
     * @param content_type MIME type string.
     * @return Reference to this HttpResponse object.
     */
    HttpResponse &setContentType(const std::string &content_type);

    /**
     * @brief Get the current Content-Type header value.
     * @return MIME type string.
     */
    std::string getContentType() const;

    /**
     * @brief Set a generic header field.
     * @param field Header name.
     * @param value Header value.
     * @return Reference to this HttpResponse object.
     */
    HttpResponse &set(const std::string &field, const std::string &value);

    /**
     * @brief Alias for set() for custom headers.
     */
    HttpResponse &setHeader(const std::string &key, const std::string &value);

    /**
     * @brief Set an Authorization header with a JWT token.
     * @param jwtToken The JWT string.
     * @return Reference to this HttpResponse object.
     */
    HttpResponse &setAuthorization(const std::string &jwtToken);

    /**
     * @brief Check if the headers have already been sent.
     * @return True if headers have been sent, false otherwise.
     */
    bool isHeaderSent() const;

    // ------------------------------------------------------------------------
    // Cookie Support
    // ------------------------------------------------------------------------

    /**
     * @brief Set a cookie to be sent with the response.
     * @param name Cookie name.
     * @param value Cookie value.
     * @param options Additional options (e.g., Path, HttpOnly).
     * @return Reference to this HttpResponse object.
     */
    HttpResponse &setCookie(const std::string &name, const std::string &value, const std::string &options);

    /**
     * @brief Clear a cookie by setting Max-Age=0.
     * @param name Cookie name.
     * @param options Optional options to include in the Set-Cookie header.
     * @return Reference to this HttpResponse object.
     */
    HttpResponse &clearCookie(const std::string &name, const std::string &options);

    // ------------------------------------------------------------------------
    // Body and Streaming
    // ------------------------------------------------------------------------

    /**
     * @brief Send a full response including headers and body.
     * @param body HttpResponse body as a string.
     */
    void send(const std::string &body);

    void renderView(const std::string &filename, const std::map<std::string, std::string> &context);

    void send(const std::string &body, const std::string &contentType);

    /**
     * @brief Send only the headers (for chunked/streaming responses).
     */
    void sendHeaders();

    /**
     * @brief Begin a streaming response by sending headers.
     * @param code HTTP status code.
     * @param contentLength Total length of body.
     * @param contentType MIME type.
     */
    void start(int code, size_t contentLength, const std::string &contentType = "application/octet-stream",
               const std::string &contentEncoding = "");

    /**
     * @brief Send a chunk of the response body.
     * @param data Pointer to data buffer.
     * @param length Size of the data.
     */
    void writeChunk(const char *data, size_t length);

    /**
     * @brief Finish the response (placeholder for potential finalization).
     */
    void finish();

    // ------------------------------------------------------------------------
    // Common Helpers
    // ------------------------------------------------------------------------
    /**
     * @brief Check if the response status code indicates success.
     * @return True if status code is 2xx, false otherwise.
     */
    bool ok() const
    {
        return status_code >= 200 && status_code < 300;
    }

    /**
     * @brief Return the raw socket descriptor.
     */
    int getSocket() const;

    /**
     * @brief Send a 401 Unauthorized JSON response.
     */
    void sendUnauthorized();

    /**
     * @brief Send a 404 Not Found JSON response.
     */
    void sendNotFound();

    /**
     * @brief Send a 500 Internal Server Error response.
     * @param msg Error message to include.
     */
    void endServerError(const std::string &msg);

    /**
     * @brief Send a JSON string/object with correct content type.
     * @param jsonString Valid JSON content.
     * @return Reference to this HttpResponse object.
     */
    HttpResponse &json(const std::string &jsonString);
    HttpResponse &json(const nlohmann::json &jsonObj);
    HttpResponse &jsonFormatted(const nlohmann::json &jsonObj);

    /**
     * @brief Send a plain text string with correct content type.
     * @param textString Text content.
     * @return Reference to this HttpResponse object.
     */
    HttpResponse &text(const std::string &textString);

    /**
     * @brief Redirect the client to another URL.
     * @param url Target URL.
     * @param statusCode HTTP status code (e.g. 302).
     * @return Reference to this HttpResponse object.
     */
    HttpResponse &redirect(const std::string &url, int statusCode);

    /**
     * @brief Sends the specified file from mounted storage to the client.
     *
     * @param path Relative path to the file (e.g. "/index.html").
     * @return true if the file was successfully sent.
     * @return false if the file was not found or an error occurred.
     */
    bool sendFile(const std::string &path);

#if defined(PICO_HTTP_ENABLE_STORAGE)
    /**
     * @brief Save the response body to a file using StorageManager.
     *
     * @param path The destination file path.
     * @return true on success, false on failure.
     */
    bool saveFile(const char *path) const;
#endif

    /**
     * @brief Apply basic variable substitution in a template.
     * @param tpl Input template with {{placeholders}}.
     * @param context Key-value map for substitution.
     * @return Final rendered template string.
     */
    std::string renderTemplate(const std::string &tpl, const std::map<std::string, std::string> &context);

    // ───── Shared Response API (Client and Server) ─────

    /**
     * @brief Get the response status code.
     * @return Status code (e.g., 200)
     */
    int getStatusCode() const
    {
        return status_code;
    }

    /**
     * @brief Get the value of a specific response header.
     * @param key Header name
     * @return Header value (empty string if not found)
     */
    std::string getHeader(const std::string &key) const
    {
        auto it = headers.find(key);
        return (it != headers.end()) ? it->second : "";
    }

    /**
     * @brief Get all response headers.
     * @return Map of header key/value pairs
     */
    const std::map<std::string, std::string> &getHeaders() const
    {
        return headers;
    }

    /**
     * @brief Get the response body.
     * @return Response body string
     */
    const std::string &getBody() const
    {
        return body;
    }

    /**
     * @brief Clear the response status, headers, and body.
     */
    void reset();

private:
    /**
     * @brief Convert an HTTP status code to its standard message.
     * @param code HTTP status code.
     * @return Corresponding message (e.g., "OK").
     */
    std::string getStatusMessage(int code);

    Tcp *tcp;                ///< Pointer to the Tcp object for socket operations
    int status_code = 200;   ///< HTTP status code
    bool headerSent = false; ///< Tracks whether headers have already been sent

    std::map<std::string, std::string> headers; ///< Response headers (server+client)
    std::vector<std::string> cookies;           ///< Set-Cookie headers (server only)

    std::string body; ///< Full response body (client-side or buffered server content)
};

#endif // HTTPRESPONSE_H
