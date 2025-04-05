#include "framework_config.h"
#include "DebugTrace.h"
TRACE_INIT(HttpClient);

#include "HttpClient.h"
#include "HttpParser.h"
#include "ChunkedDecoder.h"
#include "TcpConnectionSocket.h"
#include <sstream>
#include <cstring>

#if PICO_HTTP_CLIENT_ENABLE_TLS
#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#endif

bool HttpClient::get(const std::string& url, HttpClientResponse& response) {
    TRACE("HttpClient", "Making request to %s", url.c_str());
    // Very minimal URL parsing (http/https, host, path)
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

    if (protocol == "http") {
        return getPlain(host, path, response);
    }
#if PICO_HTTP_CLIENT_ENABLE_TLS
    else if (protocol == "https") {
        return getTls(host, path, response);
    }
#endif
    return false;
}

bool HttpClient::getPlain(const std::string& host, const std::string& path, HttpClientResponse& response) {
    TcpConnectionSocket socket;
    if (!socket.connect(host.c_str(), 80)) {
        TRACE("HttpClient", "Connection failed");
        return false;
    }

    TRACE("HttpClient", "Socket connected");
    std::ostringstream req;
    req << "GET " << path << " HTTP/1.1\r\n"
        << "Host: " << host << "\r\n"
        << "Connection: close\r\n\r\n";

    if (socket.send(req.str().c_str(), req.str().size()))
    {
        TRACE("HttpClient", "Request sent");
    } else {
        TRACE("HttpClient", "Request failed");
        return false;
    }
    TRACE("HttpClient", "Request sent");
    std::string raw;
    char buffer[1024];
    int len = 0;

    while ((len = socket.recv(buffer, sizeof(buffer))) > 0) {
        raw.append(buffer, len);
    }

    socket.close();
    TRACE("HttpClient", "Parsing response...");
    std::string headerText;
    std::string body = extractHeadersAndBody(raw, headerText);
    response.headers = HttpParser::parseHeaders(headerText);
    response.statusCode = HttpParser::parseStatusCode(headerText);
    TRACE("HttpClient", "Got status code %d", response.statusCode);

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

#if PICO_HTTP_CLIENT_ENABLE_TLS
bool HttpClient::getTls(const std::string& host, const std::string& path, HttpClientResponse& response) {
    mbedtls_net_context net;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    const char* pers = "http_client";

    mbedtls_net_init(&net);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char*)pers, strlen(pers)) != 0) {
        return false;
    }

    if (mbedtls_net_connect(&net, host.c_str(), "443", MBEDTLS_NET_PROTO_TCP) != 0) {
        return false;
    }

    if (mbedtls_ssl_config_defaults(&conf,
        MBEDTLS_SSL_IS_CLIENT,
        MBEDTLS_SSL_TRANSPORT_STREAM,
        MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
        return false;
    }

    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    if (mbedtls_ssl_setup(&ssl, &conf) != 0) {
        return false;
    }

    mbedtls_ssl_set_bio(&ssl, &net, mbedtls_net_send, mbedtls_net_recv, nullptr);

    if (mbedtls_ssl_handshake(&ssl) != 0) {
        return false;
    }

    std::ostringstream req;
    req << "GET " << path << " HTTP/1.1\r\n"
        << "Host: " << host << "\r\n"
        << "Connection: close\r\n\r\n";

    mbedtls_ssl_write(&ssl, (const unsigned char*)req.str().c_str(), req.str().size());

    std::string raw;
    char buffer[1024];
    int len = 0;
    while ((len = mbedtls_ssl_read(&ssl, (unsigned char*)buffer, sizeof(buffer))) > 0) {
        raw.append(buffer, len);
    }

    mbedtls_ssl_close_notify(&ssl);
    mbedtls_net_free(&net);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    std::string headerText;
    std::string body = extractHeadersAndBody(raw, headerText);
    response.headers = HttpParser::parseHeaders(headerText);
    response.statusCode = HttpParser::parseStatusCode(headerText);

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
#endif

std::string HttpClient::extractHeadersAndBody(const std::string& raw, std::string& headerOut) {
    size_t pos = raw.find("\r\n\r\n");
    if (pos == std::string::npos) {
        headerOut.clear();
        return raw;
    }
    headerOut = raw.substr(0, pos);
    return raw.substr(pos + 4);
}
