#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/altcp_tls.h"
#include "lwip/netif.h"
#include "FreeRTOS.h"
#include "task.h"
#include "http_client_util.h"

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    void setHost(const char* host);
    void setUrl(const char* url);
    void setTlsCertificate(const char* cert, size_t cert_size);

    void makeRequest();
    
private:
    static void httpTask(void* params);

    const char* hostname;
    const char* url;
    const uint8_t* tls_cert;
    size_t tls_cert_size;
};

#endif // HTTP_CLIENT_HPP
