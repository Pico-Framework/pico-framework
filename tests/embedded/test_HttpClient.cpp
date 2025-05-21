#include "CppUTest/TestHarness.h"
#include "HttpClient.h"

TEST_GROUP(HttpClientTest) { };

TEST(HttpClientTest, Get_ChunkedResponse_ShouldReturn200AndNonEmptyBody) {
    HttpClient client;
    HttpClientResponse response;

    bool ok = client.get("http://httpbin.org/stream/5", response);
    CHECK_TRUE(ok);
    CHECK_EQUAL(200, response.statusCode);
    CHECK_TRUE(response.body.length() > 0);
}