/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #include "pico/cyw43_arch.h"
 #include "pico/stdlib.h"
 #include "lwip/altcp_tls.h"
 
 #include "lwip/netif.h"
 
 #include "FreeRTOS.h"
 #include "task.h"
 #include "http_client_util.h"
 #include "main.hpp"
 
 #ifndef RUN_FREERTOS_ON_CORE
 #define RUN_FREERTOS_ON_CORE 0
 #endif
 
 #define TEST_TASK_PRIORITY ( tskIDLE_PRIORITY + 2UL )
 #define TEST_TASK_STACK_SIZE 1024
 
 // Using this url as we know the root cert won't change for a long time
 //#define HOST "httpbin.org"
 //#define URL_REQUEST "https://httpbin.org/bytes/2048"
   #define HOST "api.open-meteo.com"
   #define URL_REQUEST "https://api.open-meteo.com/v1/forecast?latitude=38.2324&longitude=-122.6367&current=temperature_2m&hourly=weather_code"
 // This is the PUBLIC root certificate exported from a browser
 // Note that the newlines are needed
 #define TLS_ROOT_CERT_OK "-----BEGIN CERTIFICATE-----\n\
 MIIC+jCCAn+gAwIBAgICEAAwCgYIKoZIzj0EAwIwgbcxCzAJBgNVBAYTAkdCMRAw\n\
 DgYDVQQIDAdFbmdsYW5kMRIwEAYDVQQHDAlDYW1icmlkZ2UxHTAbBgNVBAoMFFJh\n\
 c3BiZXJyeSBQSSBMaW1pdGVkMRwwGgYDVQQLDBNSYXNwYmVycnkgUEkgRUNDIENB\n\
 MR0wGwYDVQQDDBRSYXNwYmVycnkgUEkgUm9vdCBDQTEmMCQGCSqGSIb3DQEJARYX\n\
 c3VwcG9ydEByYXNwYmVycnlwaS5jb20wIBcNMjExMjA5MTEzMjU1WhgPMjA3MTEx\n\
 MjcxMTMyNTVaMIGrMQswCQYDVQQGEwJHQjEQMA4GA1UECAwHRW5nbGFuZDEdMBsG\n\
 A1UECgwUUmFzcGJlcnJ5IFBJIExpbWl0ZWQxHDAaBgNVBAsME1Jhc3BiZXJyeSBQ\n\
 SSBFQ0MgQ0ExJTAjBgNVBAMMHFJhc3BiZXJyeSBQSSBJbnRlcm1lZGlhdGUgQ0Ex\n\
 JjAkBgkqhkiG9w0BCQEWF3N1cHBvcnRAcmFzcGJlcnJ5cGkuY29tMHYwEAYHKoZI\n\
 zj0CAQYFK4EEACIDYgAEcN9K6Cpv+od3w6yKOnec4EbyHCBzF+X2ldjorc0b2Pq0\n\
 N+ZvyFHkhFZSgk2qvemsVEWIoPz+K4JSCpgPstz1fEV6WzgjYKfYI71ghELl5TeC\n\
 byoPY+ee3VZwF1PTy0cco2YwZDAdBgNVHQ4EFgQUJ6YzIqFh4rhQEbmCnEbWmHEo\n\
 XAUwHwYDVR0jBBgwFoAUIIAVCSiDPXut23NK39LGIyAA7NAwEgYDVR0TAQH/BAgw\n\
 BgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwCgYIKoZIzj0EAwIDaQAwZgIxAJYM+wIM\n\
 PC3wSPqJ1byJKA6D+ZyjKR1aORbiDQVEpDNWRKiQ5QapLg8wbcED0MrRKQIxAKUT\n\
 v8TJkb/8jC/oBVTmczKlPMkciN+uiaZSXahgYKyYhvKTatCTZb+geSIhc0w/2w==\n\
 -----END CERTIFICATE-----\n"
 
void http_task(__unused void *params) {

   printf("HTTP task started\n");
 
   static const uint8_t cert_ok[] = TLS_ROOT_CERT_OK;
   static HTTP_REQUEST_T req = {0};
   req.hostname = HOST;
   req.url = URL_REQUEST;
   req.headers_fn = http_client_header_print_fn;
   req.recv_fn = http_client_receive_print_fn;
   req.tls_config = altcp_tls_create_config_client(cert_ok, sizeof(cert_ok));
   printf("req.hostname = %s, req.url=%s\n", req.hostname, req.url);

   int pass = http_client_request_sync(cyw43_arch_async_context(), &req);
   printf("here\n");
   //altcp_tls_free_config(req.tls_config); // don't know why this is failing 
   printf("here 2\n");
   if (pass != 0) {
      printf("test failed");
   }
   printf("notify give to main\n");
   xTaskNotifyGive( main_task_handle ); // Send notification to the waiting task
   printf("notify given\n");
   printf("Test passed");
   vTaskDelete( NULL ); // Delete the task
   printf("Task deleted\n");  
 }
 
