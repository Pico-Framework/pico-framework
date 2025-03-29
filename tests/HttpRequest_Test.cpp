#include <CppUTest/TestHarness.h>

#ifdef malloc
#undef malloc
#endif
#ifdef free
#undef free
#endif
#ifdef realloc
#undef realloc
#endif
#ifdef calloc
#undef calloc
#endif
#ifdef strdup
#undef strdup
#endif
#ifdef new
#undef new
#endif

#include "HttpRequest.h"
#include <string>
#include <unordered_map>

TEST_GROUP(HttpRequest)
{
    void setup() {
        // Setup code before each test
    }

    void teardown() {
        // Cleanup after each test
    }
};

TEST(HttpRequest, ParsesHeadersCorrectly)
{
    const char* rawHeaders = "Host: localhost\r\nUser-Agent: test-agent\r\nContent-Length: 11\r\n\r\n";
    Request req(rawHeaders, "POST", "/test");
    auto headers = req.getHeaders();

    STRCMP_EQUAL("localhost", headers["host"].c_str());
    STRCMP_EQUAL("test-agent", headers["user-agent"].c_str());
    STRCMP_EQUAL("11", headers["content-length"].c_str());
}

TEST(HttpRequest, ParsesQueryParams)
{
    const char* rawHeaders = "\r\n";  // minimal
    Request req(rawHeaders, "GET", "/api?foo=bar&baz=qux");
    auto params = req.getQueryParams();

    STRCMP_EQUAL("bar", params["foo"].c_str());
    STRCMP_EQUAL("qux", params["baz"].c_str());
}

TEST(HttpRequest, ParsesCookies)
{
    const char* rawHeaders = "Cookie: session=abc123; theme=dark\r\n\r\n";
    Request req(rawHeaders, "GET", "/");
    auto cookies = req.getCookies();

    STRCMP_EQUAL("abc123", cookies["session"].c_str());
    STRCMP_EQUAL("dark", cookies["theme"].c_str());
}

TEST(HttpRequest, HandlesMissingHeadersGracefully)
{
    const char* rawHeaders = "\r\n";
    Request req(rawHeaders, "GET", "/");
    auto headers = req.getHeaders();

    CHECK(headers.empty());
    STRCMP_EQUAL("", req.getHeader("does-not-exist").c_str());
}
