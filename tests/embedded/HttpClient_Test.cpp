#include "CppUTest/TestHarness.h"
#include "http-client/HttpClient.hpp"

TEST_GROUP(HttpClientTest) { };

TEST(HttpClientTest, Get_ChunkedResponse_ShouldReturn200AndNonEmptyBody)
{
    HttpClient client;
    HttpResponse response;

    bool ok = client.get("https://httpbin.org/stream/3", response);

    CHECK_TRUE(ok);
    LONGS_EQUAL(200, response.statusCode);
    CHECK_TRUE(response.body.length() > 0);
    STRCMP_CONTAINS("{", response.body.c_str());  // crude check for JSON
}