/**
 * @file https_example.cpp
 * @brief Example showing how to enable HTTPS in PicoFramework
 */

#include "framework/FrameworkApp.h"

// Example server certificate (you'd load this from storage or embed it)
const char* SERVER_CERT_PEM = R"(
-----BEGIN CERTIFICATE-----
MIICljCCAX4CCQDKOGJQUuSHWTANBgkqhkiG9w0BAQsFADCBjTELMAkGA1UEBhMC
VVMxCzAJBgNVBAgMAlRYMQ8wDQYDVQQHDAZEYWxsYXMxEjAQBgNVBAoMCVBpY29G
cmFtZTEQMA4GA1UECwwHRXhhbXBsZTEaMBgGA1UEAwwRcGljby1mcmFtZXdvcmsu
Y29tMR4wHAYJKoZIhvcNAQkBFg9pbmZvQGV4YW1wbGUuY29tMB4XDTIzMDEwMTAw
MDAwMFoXDTI0MDEwMTAwMDAwMFowgY0xCzAJBgNVBAYTAlVTMQswCQYDVQQIDAJU
WDEPMA0GA1UEBwwGRGFsbGFzMRIwEAYDVQQKDAlQaWNvRnJhbWUxEDAOBgNVBAsM
B0V4YW1wbGUxGjAYBgNVBAMMEXBpY28tZnJhbWV3b3JrLmNvbTEeMBwGCSqGSIb3
DQEJARYPaW5mb0BleGFtcGxlLmNvbTCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkC
gYEAuVqVeEzGIQnfp2lDQxs2BYxIuTiuzk1boMPpf7wer4Exq3JQYi5wLMzZiP/U
VTBuHBOonV8Af1DlzfNcx+jMLVwBAoGBANYFAwVuxvl+pIlrjMZHVsgHwX7fEIN9
wRiHkFIoiEXIwkBi0s4+3ZbvztRzrHRiH+iN+d+sjyio
-----END CERTIFICATE-----
)";

const char* SERVER_KEY_PEM = R"(
-----BEGIN PRIVATE KEY-----
MIICdgIBADANBgkqhkiG9w0BAQEFAASCAmAwggJcAgEAAoGBALlalXhMxiEJ36dp
Q0MbNgWMSLk4rs5NW6DD6X+8Hq+BMatyUGIucCzM2Yj/1FUwbhwTqJ1fAH9Q5c3z
XMfozC1cAQKBgQDWBQMFbsb5fqSJa4zGR1bIB8F+3xCDfcEYh5BSKIhFyMJAYtLO
Pt2W787Uc6x0Yh/ojfnfrI8oqA==
-----END PRIVATE KEY-----
)";

class HttpsApp : public FrameworkApp
{
public:
    HttpsApp(int port) : FrameworkApp(port, "HttpsApp", 2048, 1)
    {
        // Enable HTTPS with certificate and key
        server.enableTLS(SERVER_CERT_PEM, SERVER_KEY_PEM);
    }

    void initRoutes() override
    {
        router.addRoute("GET", "/", [](HttpRequest &req, HttpResponse &res, const auto &)
        {
            res.send("Hello from HTTPS server!");
        });

        router.addRoute("GET", "/secure", [](HttpRequest &req, HttpResponse &res, const auto &)
        {
            res.json({
                {"message", "This is a secure HTTPS endpoint"},
                {"protocol", "HTTPS"},
                {"encrypted", true}
            });
        });
    }
};

int main()
{
    // Create HTTPS server on port 443 (standard HTTPS port)
    HttpsApp app(443);
    app.start();
    
    // Keep the main thread alive
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    return 0;
}
