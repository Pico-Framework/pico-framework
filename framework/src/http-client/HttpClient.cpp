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

#if PICO_HTTP_CLIENT_ENABLE_TLS
void HttpClient::setRootCACertificate(const std::string& cert) {
    rootCACert = cert;
}
#endif

static int mbedtls_send_callback(void* ctx, const unsigned char* buf, size_t len) {
    TcpConnectionSocket* socket = static_cast<TcpConnectionSocket*>(ctx);
    return socket->send(reinterpret_cast<const char*>(buf), len);
}

static int mbedtls_recv_callback(void* ctx, unsigned char* buf, size_t len) {
    TcpConnectionSocket* socket = static_cast<TcpConnectionSocket*>(ctx);
    return socket->recv(reinterpret_cast<char*>(buf), len);
}

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
        printf("HttpClient: Connection failed\n");
        return false;
    }

    printf("HttpClient: Socket connected\n");

    std::ostringstream req;
    req << "GET " << path << " HTTP/1.1\r\n"
        << "Host: " << host << "\r\n"
        << "Connection: close\r\n\r\n";

    if (!socket.send(req.str().c_str(), req.str().size())) {
        printf("HttpClient: Request failed\n");
        return false;
    }

    char buffer[1024];
    std::string raw;
    int len;

    while ((len = socket.recv(buffer, sizeof(buffer))) > 0) {
        raw.append(buffer, len);
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

#if PICO_HTTP_CLIENT_ENABLE_TLS
bool HttpClient::getTls(const std::string& host, const std::string& path, HttpClientResponse& response) {
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    const char* pers = "pico_https_client";
    TcpConnectionSocket socket;

    if (!socket.connect(host.c_str(), 443)) {
        printf("TLS: Socket connection failed\n");
        return false;
    }

    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                               reinterpret_cast<const unsigned char*>(pers), strlen(pers)) != 0) {
        printf("TLS: Failed to seed DRBG\n");
        return false;
    }

    if (mbedtls_ssl_config_defaults(&conf,
                                    MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
        printf("TLS: SSL config defaults failed\n");
        return false;
    }

    mbedtls_ssl_conf_authmode(&conf, ALTCP_MBEDTLS_AUTHMODE);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    if (mbedtls_ssl_setup(&ssl, &conf) != 0) {
        printf("TLS: SSL setup failed\n");
        return false;
    }

    mbedtls_ssl_set_bio(&ssl, &socket, mbedtls_send_callback, mbedtls_recv_callback, nullptr);
    mbedtls_ssl_set_hostname(&ssl, host.c_str());

    while (true) {
        int ret = mbedtls_ssl_handshake(&ssl);
        if (ret == 0) break;
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            printf("TLS: Handshake failed: -0x%x\n", -ret);
            return false;
        }
    }

    std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    if (mbedtls_ssl_write(&ssl, reinterpret_cast<const unsigned char*>(request.c_str()), request.size()) <= 0) {
        printf("TLS: Write failed\n");
        return false;
    }

    std::string raw;
    char buf[1024];
    int len;

    while ((len = mbedtls_ssl_read(&ssl, reinterpret_cast<unsigned char*>(buf), sizeof(buf))) > 0) {
        raw.append(buf, len);
    }

    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

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
