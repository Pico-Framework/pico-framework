#include "http_client.hpp"
#include <cstdio>

// Default certificate
static const uint8_t DEFAULT_TLS_CERT[] = 
    "-----BEGIN CERTIFICATE-----\n"
    "MIIC+jCCAn+gAwIBAgICEAAwCgYIKoZIzj0EAwIwgbcxCzAJBgNVBAYTAkdCMRAw\n"
    "DgYDVQQIDAdFbmdsYW5kMRIwEAYDVQQHDAlDYW1icmlkZ2UxHTAbBgNVBAoMFFJh\n"
    "c3BiZXJyeSBQSSBMaW1pdGVkMRwwGgYDVQQLDBNSYXNwYmVycnkgUEkgRUNDIENB\n"
    "MR0wGwYDVQQDDBRSYXNwYmVycnkgUEkgUm9vdCBDQTEmMCQGCSqGSIb3DQEJARYX\n"
    "c3VwcG9ydEByYXNwYmVycnlwaS5jb20wIBcNMjExMjA5MTEzMjU1WhgPMjA3MTEx\n"
    "MjcxMTMyNTVaMIGrMQswCQYDVQQGEwJHQjEQMA4GA1UECAwHRW5nbGFuZDEdMBsG\n"
    "A1UECgwUUmFzcGJlcnJ5IFBJIExpbWl0ZWQxHDAaBgNVBAsME1Jhc3BiZXJyeSBQ\n"
    "SSBFQ0MgQ0ExJTAjBgNVBAMMHFJhc3BiZXJyeSBQSSBJbnRlcm1lZGlhdGUgQ0Ex\n"
    "JjAkBgkqhkiG9w0BCQEWF3N1cHBvcnRAcmFzcGJlcnJ5cGkuY29tMHYwEAYHKoZI\n"
    "zj0CAQYFK4EEACIDYgAEcN9K6Cpv+od3w6yKOnec4EbyHCBzF+X2ldjorc0b2Pq0\n"
    "N+ZvyFHkhFZSgk2qvemsVEWIoPz+K4JSCpgPstz1fEV6WzgjYKfYI71ghELl5TeC\n"
    "byoPY+ee3VZwF1PTy0cco2YwZDAdBgNVHQ4EFgQUJ6YzIqFh4rhQEbmCnEbWmHEo\n"
    "XAUwHwYDVR0jBBgwFoAUIIAVCSiDPXut23NK39LGIyAA7NAwEgYDVR0TAQH/BAgw\n"
    "BgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwCgYIKoZIzj0EAwIDaQAwZgIxAJYM+wIM\n"
    "PC3wSPqJ1byJKA6D+ZyjKR1aORbiDQVEpDNWRKiQ5QapLg8wbcED0MrRKQIxAKUT\n"
    "v8TJkb/8jC/oBVTmczKlPMkciN+uiaZSXahgYKyYhvKTatCTZb+geSIhc0w/2w==\n"
    "-----END CERTIFICATE-----\n";

HttpClient::HttpClient()
    : hostname("api.open-meteo.com"),
      url("https://api.open-meteo.com/v1/forecast?latitude=38.2324&longitude=-122.6367&current=temperature_2m&hourly=weather_code"),
      tls_cert(DEFAULT_TLS_CERT),
      tls_cert_size(sizeof(DEFAULT_TLS_CERT)) {}

HttpClient::~HttpClient() {}

void HttpClient::setHost(const char* host) {
    hostname = host;
}

void HttpClient::setUrl(const char* new_url) {
    url = new_url;
}

void HttpClient::setTlsCertificate(const char* cert, size_t cert_size) {
    tls_cert = reinterpret_cast<const uint8_t*>(cert);
    tls_cert_size = cert_size;
}

void HttpClient::makeRequest() {
    xTaskCreate(httpTask, "HTTP Task", 1024, this, tskIDLE_PRIORITY + 2, nullptr);
}

void HttpClient::httpTask(void* params) {
    HttpClient* client = static_cast<HttpClient*>(params);
    printf("HTTP task started\n");

    HTTP_REQUEST_T req = {0};
    req.hostname = client->hostname;
    req.url = client->url;
    req.headers_fn = http_client_header_print_fn;
    req.recv_fn = http_client_receive_print_fn;
    req.tls_config = altcp_tls_create_config_client(client->tls_cert, client->tls_cert_size);

    printf("req.hostname = %s, req.url = %s\n", req.hostname, req.url);

    int pass = http_client_request_sync(cyw43_arch_async_context(), &req);
    
    if (pass != 0) {
        printf("HTTP request failed\n");
    } else {
        printf("HTTP request successful\n");
    }

    vTaskDelete(nullptr);
}
