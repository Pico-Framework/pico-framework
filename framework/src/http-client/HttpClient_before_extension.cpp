#include "HttpClient.h"
#include "HttpParser.h"
#include "ChunkedDecoder.h"
#include "TcpConnectionSocket.h"

#include <sstream>
#include <cstring>
bool HttpClient::get(const std::string& url, HttpClientResponse& response) {
    printf("HttpClient: Making request to %s\n", url.c_str());

    std::string protocol, host, path;
    size_t proto_pos = url.find("://");
    if (proto_pos == std::string::npos) return false;

    protocol = url.substr(0, proto_pos);
    size_t host_start = proto_pos + 3;
    size_t path_start = url.find('/', host_start);
    if (path_start == std::string::npos) {
        host = url.substr(host_start);
        path = "/";
    } else {
        host = url.substr(host_start, path_start - host_start);
        path = url.substr(path_start);
    }

    printf("HttpClient: Protocol: %s, Host: %s, Path: %s\n", protocol.c_str(), host.c_str(), path.c_str());

    TcpConnectionSocket socket;
    bool connected = false;

    if (protocol == "https") {
        socket.setRootCACertificate(rootCACert);
        socket.setHostname(host.c_str());
        connected = socket.connectTls(host.c_str(), 443);
    } else if (protocol == "http") {
        connected = socket.connect(host.c_str(), 80);
    } else {
        printf("HttpClient: Unsupported protocol %s\n", protocol.c_str());
        return false;
    }

    if (!connected) {
        printf("HttpClient: Connection failed\n");
        return false;
    }

    printf("HttpClient: Connected to %s over %s\n", host.c_str(), protocol.c_str());

    std::ostringstream req;
    req << "GET " << path << " HTTP/1.1\r\n"
        << "Host: " << host << "\r\n\r\n"
        << "User-Agent: PicoHttpClient/1.0\r\n"
        << "Connection: close\r\n\r\n";

    std::string request = req.str();
    printf("[HttpClient] Request:\n%s\n", request.c_str());

    int sent = socket.send(request.c_str(), request.size() + 1);
    if (sent < 0) {
        printf("HttpClient: Send failed %d\n", sent);
        return false;
    }

    printf("HttpClient: Request sent\n");

    char buf[1024];
    std::string raw;
    int len;
    while ((len = socket.recv(buf, sizeof(buf))) > 0) {
        raw.append(buf, len);
    }

    socket.close();

    std::string headerText;
    std::string body = extractHeadersAndBody(raw, headerText);
    response.headers = HttpParser::parseHeaders(headerText);
    response.statusCode = HttpParser::parseStatusCode(headerText);
    printf("HttpClient: Got status code %d\n", response.statusCode);

    auto it = response.headers.find("transfer-encoding");
    if (it != response.headers.end() && it->second == "chunked") {
        ChunkedDecoder decoder;
        decoder.feed(body);
        response.body = decoder.getDecoded();
    } else {
        response.body = body;
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

