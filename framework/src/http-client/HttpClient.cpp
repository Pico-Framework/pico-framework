#include "HttpClient.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpParser.h"
#include "ChunkedDecoder.h"
#include "TcpConnectionSocket.h"

#include <sstream>
#include <cstring>

bool HttpClient::gets(const std::string& uri, HttpResponse& response) {
    HttpRequest request;

    const std::string::size_type proto_end = uri.find("://");
    if (proto_end == std::string::npos) return false;

    std::string protocol = uri.substr(0, proto_end);
    std::string rest = uri.substr(proto_end + 3); // Skip "://"

    std::string::size_type path_start = rest.find('/');
    std::string host, path;

    if (path_start != std::string::npos) {
        host = rest.substr(0, path_start);
        path = rest.substr(path_start);
    } else {
        host = rest;
        path = "/";
    }

    request.setProtocol(protocol)
           .setHost(host)
           .setUri(path)
           .setMethod("GET")
           .setUserAgent("PicoFramework/1.0");

    return get(request, response);
}

bool HttpClient::get(const HttpRequest& request, HttpResponse& response) {
    const std::string& protocol = request.getProtocol();
    const std::string& host     = request.getHost();
    const std::string& path     = request.getUri();
    const std::string& method   = request.getMethod();
    const std::string& body     = request.getBody();
    const auto& headers         = request.getHeaders();

    const bool useTls = (protocol == "https");
    const uint16_t port = useTls ? 443 : 80;

    printf("HttpClient: Making request to %s\n", host.c_str());
    printf("HttpClient: Protocol: %s, Port: %d, Host: %s, Path: %s\n", protocol.c_str(), port, host.c_str(), path.c_str());
    printf("HttpClient: Method: %s\n", method.c_str());
    printf("HttpClient: Body: %s\n", body.c_str());
    printf("HttpClient: User Headers:\n");

    TcpConnectionSocket socket;
    if (useTls){
        socket.setRootCACertificate(rootCACert);
        socket.setHostname(host.c_str()); // required for SNI
    }
    if (!socket.connect(host.c_str(), port, useTls)) {
        return false;
    }

    std::ostringstream req;
    req << method << " " << path << " HTTP/1.1\r\n";
    req << "Host: " << host << "\r\n";

    for (const auto& [key, value] : headers) {
        req << key << ": " << value << "\r\n";
    }

    if (!body.empty()) {
        req << "Content-Length: " << body.length() << "\r\n";
    }

    req << "\r\n";
    if (!body.empty()) {
        req << body;
    }

    std::string requestStr = req.str();
    if (!socket.send(requestStr.c_str(), requestStr.length())) {
        return false;
    }

    auto [rawHeader, leftover] = HttpParser::receiveHeaderAndLeftover(socket);
    if (rawHeader.empty()) {
        return false;
    }

    response.setStatus(HttpParser::parseStatusCode(rawHeader));

    auto parsedHeaders = HttpParser::parseHeaders(rawHeader);
    for (const auto& [key, value] : parsedHeaders) {
        response.setHeader(key, value);
    }

    std::string bodyData;
    if (!HttpParser::receiveBody(socket, parsedHeaders, leftover, bodyData)) {
        return false;
    }

    auto transferEncoding = response.getHeader("Transfer-Encoding");
    if (transferEncoding == "chunked") {
        ChunkedDecoder decoder;
        decoder.feed(bodyData);
        response.setBody(decoder.getDecoded());
    } else {
        response.setBody(bodyData);
    }

    return true;
}

std::string HttpClient::extractHeadersAndBody(const std::string& raw, std::string& headerOut) {
    size_t pos = raw.find("\r\n\r\n");
    if (pos == std::string::npos) {
        headerOut.clear();
        return raw;
    }
    headerOut = raw.substr(0, pos);
    return raw.substr(pos + 4);
}

