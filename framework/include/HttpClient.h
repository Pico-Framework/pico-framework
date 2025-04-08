#pragma once
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <string>
#include <map>
class HttpClient {
public:
    /**
     * @brief Perform a GET request using a pre-built HttpRequest.
     * @param request A fully constructed HttpRequest (with method, host, uri, etc.)
     * @param response An HttpResponse that will be populated
     * @return true if the request succeeded and response was parsed
     */
    bool get(const HttpRequest& request, HttpResponse& response);

    /**
     * @brief Perform a simple GET request by URL (for convenience).
     * @param url Full URL (e.g., "http://example.com/path")
     * @param response HttpResponse object to populate
     * @return true on success
     */
    bool gets(const std::string& url, HttpResponse& response);

    /**
     * @brief Set the Root CA Certificate used for TLS validation.
     * 
     * @param cert PEM-encoded root CA certificate.
     */
    void setRootCACertificate(const std::string& cert){
        rootCACert = cert;
    }

private:
    bool request(const std::string& method,
                 const std::string& url,
                 const std::map<std::string, std::string>& headers,
                 const std::string& body,
                 HttpResponse& response);

    bool getPlain(const std::string& host, const std::string& path, HttpResponse& response);

    std::string rootCACert;

    std::string extractHeadersAndBody(const std::string& raw, std::string& headerOut);
};
