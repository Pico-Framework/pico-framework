#pragma once

#include <string>
#include <unordered_map>

struct HttpClientResponse {
    int statusCode = 0;
    std::string body;
    std::unordered_map<std::string, std::string> headers;

    void clear() {
        statusCode = 0;
        body.clear();
        headers.clear();
    }
};


class HttpClient {
public:
    /**
     * @brief Perform a GET request to the given URL.
     * 
     * @param url The full URL (http:// or https://).
     * @param response Output parameter to hold status, headers, and body.
     * @return true on success, false on failure.
     */
    bool get(const std::string& url, HttpClientResponse& response);

private:
    bool request(const std::string& method,
                 const std::string& url,
                 const std::unordered_map<std::string, std::string>& headers,
                 const std::string& body,
                 HttpClientResponse& response);

    bool getPlain(const std::string& host, const std::string& path, HttpClientResponse& response);

    std::string rootCACert;
    
#if PICO_HTTP_CLIENT_ENABLE_TLS
    bool getTls(const std::string& host, const std::string& path, HttpClientResponse& response);
    /**
     * @brief Set the Root CA Certificate used for TLS validation.
     * 
     * @param cert PEM-encoded root CA certificate.
     */
    void setRootCACertificate(const std::string& cert);
#endif

    std::string extractHeadersAndBody(const std::string& raw, std::string& headerOut);
};
