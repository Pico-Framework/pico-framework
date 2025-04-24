#include "http/HttpClient.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"
#include "http/HttpParser.h"
#include "http/ChunkedDecoder.h"
#include "network/Tcp.h"

#include <sstream>
#include <cstring>

#include "framework_config.h"
#include "DebugTrace.h"
TRACE_INIT(HttpClient)

bool HttpClient::sendRequest(const HttpRequest &request, HttpResponse &response)
{

    response.reset(); // Clear any prior response data

    const std::string &protocol = request.getProtocol();
    const std::string &host = request.getHost();
    const std::string &path = request.getUri();
    const std::string &method = request.getMethod();
    const std::string &body = request.getBody();
    const auto &headers = request.getHeaders();
    const std::string &cert = request.getRootCACertificate();

    const bool useTls = (protocol == "https");
    const uint16_t port = useTls ? 443 : 80;

    Tcp socket;

    TRACE("Connecting to %s:%d\n", host.c_str(), port);
    TRACE("Using TLS: %s\n", useTls ? "true" : "false");
    TRACE("Root CA: %s\n", cert.empty() ? "none" : "set");
    TRACE("Method: %s\n", method.c_str());
    TRACE("Path: %s\n", path.c_str());
    TRACE("Body: %s\n", body.c_str());
    TRACE("Headers:\n");
    for (const auto &[key, value] : headers)
    {
        TRACE("  %s: %s\n", key.c_str(), value.c_str());
    }

    if (useTls)
    {
        if (!cert.empty())
        {
            socket.setRootCACertificate(cert);
        }
        socket.setHostname(host.c_str()); // required for SNI
    }

    if (!socket.connect(host.c_str(), port, useTls))
    {
        return false;
    }

    std::ostringstream req;
    req << method << " " << path << " HTTP/1.1\r\n";
    req << "Host: " << host << "\r\n";

    for (const auto &[key, value] : headers)
    {
        req << key << ": " << value << "\r\n";
    }

    if (!body.empty())
    {
        req << "Content-Length: " << body.length() << "\r\n";
    }

    req << "\r\n";
    if (!body.empty())
    {
        req << body;
    }

    const std::string requestStr = req.str();
    if (!socket.send(requestStr.c_str(), requestStr.length()))
    {
        return false;
    }

    auto [rawHeader, leftover] = HttpParser::receiveHeaderAndLeftover(socket);
    if (rawHeader.empty())
    {
        return false;
    }

    TRACE("Raw header: %s\n", rawHeader.c_str());
    TRACE("Leftover: %s\n", leftover.c_str());

    response.setStatus(HttpParser::parseStatusCode(rawHeader));
    const auto parsedHeaders = HttpParser::parseHeaders(rawHeader);
    for (const auto &[key, value] : parsedHeaders)
    {
        response.setHeader(key, value);
    }

    std::string bodyData;
    bool truncated = false;
    
    if (!HttpParser::receiveBody(socket, parsedHeaders, leftover, bodyData, MAX_HTTP_BODY_LENGTH, &truncated))
    {
        return false;
    }
    response.setBody(bodyData);
    if (truncated) response.markBodyTruncated();
    return true;
}
