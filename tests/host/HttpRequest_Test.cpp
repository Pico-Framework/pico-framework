#include <CppUTest/TestHarness.h>

// Needed in this order to avoid problems with memory allocation
#include "mocks/mem_redefines.h"

#include "HttpRequest.h"
#include <string>
#include <unordered_map>



TEST_GROUP(HttpRequest)
{
    void setup() {
        // Setup code before each test (if needed)
    }

    void teardown() {
        // Cleanup after each test (if needed)
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
    const char* rawHeaders = "\r\n";  // minimal headers
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

//
// Additional tests to extend coverage
//

TEST(HttpRequest, StoresMethodAndPath)
{
    const char* rawHeaders = "\r\n";
    Request req(rawHeaders, "PUT", "/resource?id=123");
    // Assuming accessor methods exist for method and path:
    STRCMP_EQUAL("PUT", req.getMethod().c_str());
    STRCMP_EQUAL("/resource", req.getPath().c_str());
    auto query = req.getQueryParams();
    STRCMP_EQUAL("123", query["id"].c_str());
}

TEST(HttpRequest, ParsesContentLength)
{
    const char* rawHeaders = "Content-Length: 42\r\n\r\n";
    Request req(rawHeaders, "GET", "/test");
    // getContentLength should parse the header value as integer
    LONGS_EQUAL(42, req.getContentLength());
}

TEST(HttpRequest, MissingContentLengthReturnsZero)
{
    const char* rawHeaders = "Host: example.com\r\n\r\n";
    Request req(rawHeaders, "GET", "/test");
    LONGS_EQUAL(0, req.getContentLength());
}

TEST(HttpRequest, DuplicateHeadersOverwrite)
{
    const char* rawHeaders = "X-Test: first\r\nX-Test: second\r\n\r\n";
    Request req(rawHeaders, "GET", "/");
    auto headers = req.getHeaders();
    // The later header value should overwrite the earlier one
    STRCMP_EQUAL("second", headers["x-test"].c_str());
}

TEST(HttpRequest, TrimsAndRemovesQuotesInHeaders)
{
    const char* rawHeaders = "Content-Type: \" application/json \"\r\n\r\n";
    Request req(rawHeaders, "GET", "/test");
    auto headers = req.getHeaders();
    // Expect quotes to be removed and extra whitespace trimmed
    STRCMP_EQUAL("application/json", headers["content-type"].c_str());
}

TEST(HttpRequest, MissingCookieReturnsEmpty)
{
    const char* rawHeaders = "Cookie: a=1; b=2\r\n\r\n";
    Request req(rawHeaders, "GET", "/");
    STRCMP_EQUAL("", req.getCookie("nonexistent").c_str());
}

TEST(HttpRequest, ParsesEncodedQueryParams)
{
    const char* rawHeaders = "\r\n";
    Request req(rawHeaders, "GET", "/search?q=hello%20world&lang=en");
    auto queryParams = req.getQueryParams();
    // Assuming parseUrlEncoded decodes percent encoding
    STRCMP_EQUAL("hello world", queryParams["q"].c_str());
    STRCMP_EQUAL("en", queryParams["lang"].c_str());
}

TEST(HttpRequest, ParsesFormParams)
{
    const char* rawHeaders = "Content-Length: 27\r\n\r\n";
    Request req(rawHeaders, "POST", "/submit");
    // Manually set the body to a URL-encoded form string
    req.setBody("name=John+Doe&age=30");
    auto formParams = req.getFormParams();
    // Assuming '+' is decoded as space
    STRCMP_EQUAL("John Doe", formParams["name"].c_str());
    STRCMP_EQUAL("30", formParams["age"].c_str());
}

TEST(HttpRequest, EmptyFormParams)
{
    const char* rawHeaders = "Content-Length: 0\r\n\r\n";
    Request req(rawHeaders, "POST", "/submit");
    req.setBody("");
    auto formParams = req.getFormParams();
    CHECK(formParams.empty());
}

TEST(HttpRequest, DecodesPlusInFormParams)
{
    const char* rawHeaders = "Content-Length: 19\r\n\r\n";
    Request req(rawHeaders, "POST", "/submit");
    req.setBody("name=Jane+Doe&city=NYC");
    auto formParams = req.getFormParams();
    STRCMP_EQUAL("Jane Doe", formParams["name"].c_str());
    STRCMP_EQUAL("NYC", formParams["city"].c_str());
}

TEST(HttpRequest, DetectsMultipart)
{
    const char* rawHeaders = "Content-Type: multipart/form-data; boundary=--XYZ\r\n\r\n";
    Request req(rawHeaders, "POST", "/upload");
    // Assuming isMultipart() returns true when the Content-Type header contains multipart/form-data
    CHECK_TRUE(req.isMultipart());
}

TEST(HttpRequest, NonMultipartDetection)
{
    const char* rawHeaders = "Content-Type: application/x-www-form-urlencoded\r\n\r\n";
    Request req(rawHeaders, "POST", "/submit");
    CHECK_FALSE(req.isMultipart());
}
