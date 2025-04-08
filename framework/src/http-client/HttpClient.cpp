#include "HttpClient.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpParser.h"
#include "ChunkedDecoder.h"
#include "TcpConnectionSocket.h"

#include <sstream>
#include <cstring>
bool HttpClient::get(const HttpRequest& request, HttpResponse& response) {
    const std::string& protocol = request.getProtocol();
    const std::string& host     = request.getHost();
    const std::string& path     = request.getUri();
    const std::string& method   = request.getMethod();
    const std::string& body     = request.getBody();
    const auto& headers         = request.getHeaders();

    const bool useTls = (protocol == "https");
    const uint16_t port = useTls ? 443 : 80;

    TcpConnectionSocket socket;
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

    std::string rawHeader;
    if (!HttpParser::receiveHeaders(socket, rawHeader)) {
        return false;
    }

    response.setStatus(HttpParser::parseStatusCode(rawHeader));

    auto parsedHeaders = HttpParser::parseHeaders(rawHeader);
    for (const auto& [key, value] : parsedHeaders) {
        response.setHeader(key, value);
    }

    std::string bodyData;
    if (!HttpParser::receiveBody(socket, parsedHeaders, bodyData)) {
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

