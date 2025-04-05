#include "HttpClient.hpp"
#include "TcpConnectionSocket.h"
#include <cstring>

#ifdef PICO_HTTP_CLIENT_ENABLE_TLS
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#endif

static std::string extractBody(const std::string& response) {
    size_t pos = response.find("\r\n\r\n");
    return (pos != std::string::npos) ? response.substr(pos + 4) : "";
}

bool HttpClient::get(const std::string& url, std::string& outBody) {
    UrlComponents parts;
    if (!UrlUtils::parse(url, parts)) return false;

#ifdef PICO_HTTP_CLIENT_ENABLE_TLS
    if (parts.protocol == "https") {
        return getTls(parts.host, parts.port, parts.path, outBody);
    }
#endif
    return getPlain(parts.host, parts.port, parts.path, outBody);
}

bool HttpClient::getPlain(const std::string& host, uint16_t port, const std::string& path, std::string& outBody) {
    TcpConnectionSocket socket;
    if (!socket.connect(host.c_str(), port)) return false;

    std::string request =
        "GET " + path + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n\r\n";

    if (!socket.send(request)) return false;

    std::string response;
    char buffer[1024];
    int received;

    while ((received = socket.receive(buffer, sizeof(buffer))) > 0) {
        response.append(buffer, received);
    }

    socket.close();
    outBody = extractBody(response);
    return true;
}

#ifdef PICO_HTTP_CLIENT_ENABLE_TLS
bool HttpClient::getTls(const std::string& host, uint16_t port, const std::string& path, std::string& outBody) {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_net_context net;

    const char* pers = "http_client_tls";

    mbedtls_net_init(&net);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                               reinterpret_cast<const unsigned char*>(pers), strlen(pers)) != 0)
        return false;

    if (mbedtls_net_connect(&net, host.c_str(), std::to_string(port).c_str(), MBEDTLS_NET_PROTO_TCP) != 0)
        return false;

    if (mbedtls_ssl_config_defaults(&conf,
                                    MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT) != 0)
        return false;

    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE); // optional: change for cert verification
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

    if (mbedtls_ssl_setup(&ssl, &conf) != 0) return false;

    mbedtls_ssl_set_hostname(&ssl, host.c_str());
    mbedtls_ssl_set_bio(&ssl, &net, mbedtls_net_send, mbedtls_net_recv, nullptr);

    int ret;
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            return false;
    }

    std::string request =
        "GET " + path + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n\r\n";

    ret = mbedtls_ssl_write(&ssl, reinterpret_cast<const unsigned char*>(request.c_str()), request.length());
    if (ret < 0) return false;

    std::string response;
    char buffer[1024];

    do {
        ret = mbedtls_ssl_read(&ssl, reinterpret_cast<unsigned char*>(buffer), sizeof(buffer) - 1);
        if (ret > 0) response.append(buffer, ret);
    } while (ret > 0);

    outBody = extractBody(response);

    mbedtls_ssl_close_notify(&ssl);
    mbedtls_net_free(&net);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return true;
}
#endif
