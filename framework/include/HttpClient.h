#pragma once
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <string>
#include <map>
class HttpClient {
public:
    /**
     * @brief Perform a GET/POST/PUT/DELETE request using a pre-built HttpRequest.
     * @param request A fully constructed HttpRequest (with method, host, uri, etc.)
     * @param response An HttpResponse that will be populated
     * @return true if the request succeeded and response was parsed
     */
    bool get(const HttpRequest& request, HttpResponse& response);
    bool post(const HttpRequest& request, HttpResponse& response);
    bool put(const HttpRequest& request, HttpResponse& response);
    bool delete_(const HttpRequest& request, HttpResponse& response);

    bool sendRequest(const HttpRequest& request, HttpResponse& response); // common helper

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

    std::string rootCACert;

    std::string extractHeadersAndBody(const std::string& raw, std::string& headerOut);
};
