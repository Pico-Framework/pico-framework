#include "CppUTest/TestHarness.h"

#include "mocks/mem_redefines.h"  // Include memory redefinitions for testing

#include "HttpResponse.h"
#include <cstring>
#include <sstream>
#include <map>
#include <string>
#define __UNIT_TEST_HEADER__
#include "sockets.h"  // Must be after UNIT_TEST is defined

#define g_lwip_sent_buffer __test_lwip_buffer()

TEST_GROUP(HttpResponse)
{
    void setup() {
        g_lwip_sent_buffer.clear();
    }
    void teardown() {
        std::string().swap(g_lwip_sent_buffer);  // Clear the buffer after each test
    }
};

TEST(HttpResponse, SendMethodSendsHeadersAndBody)
{
    Response resp(1);
    std::string body = "Hello World";
    // Calling send() should send headers (if not already sent) and then the body.
    resp.send(body);
    
    // Verify that the header includes the status line and default headers.
    STRCMP_CONTAINS("HTTP/1.1 200 OK", g_lwip_sent_buffer.c_str());
    
    // Check that Content-Length is set to the body length.
    char expectedContentLength[50];
    snprintf(expectedContentLength, sizeof(expectedContentLength), "Content-Length: %zu", body.size());

    STRCMP_CONTAINS(expectedContentLength, g_lwip_sent_buffer.c_str());
    
    // Check that default Content-Type is set.
    STRCMP_CONTAINS("Content-Type: text/html", g_lwip_sent_buffer.c_str());
    
    // Verify that the body is sent.
    STRCMP_CONTAINS("Hello World", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, SetAndGetAuthorizationHeader)
{
    Response resp(1);
    resp.setAuthorization("mytoken");
    resp.send("Test");
    
    // Verify that the Authorization header is present.
    STRCMP_CONTAINS("Authorization: Bearer mytoken", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, SetCookieAndClearCookie)
{
    Response resp(1);
    resp.setCookie("token", "abc123", "HttpOnly; Path=/");
    resp.clearCookie("token", "Path=/");
    resp.send("Cookie Test");
    
    // Check that both the cookie and the clear cookie headers are present.
    STRCMP_CONTAINS("Set-Cookie: token=abc123; HttpOnly; Path=/", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("Set-Cookie: token=; Max-Age=0; Path=/", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, SendHeadersOnlySendsOnce)
{
    Response resp(1);
    resp.set("X-Test", "value");
    resp.sendHeaders();
    
    size_t len = g_lwip_sent_buffer.size();
    resp.sendHeaders(); // Second call should not re-send headers.
    LONGS_EQUAL(len, g_lwip_sent_buffer.size());
    
    // Check for the custom header.
    STRCMP_CONTAINS("X-Test: value", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, StartMethodSendsInitialHeaders)
{
    Response resp(1);
    resp.start(404, 10, "application/json");
    
    STRCMP_CONTAINS("HTTP/1.1 404 Not Found", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("Content-Length: 10", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("Content-Type: application/json", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, WriteChunkSendsData)
{
    Response resp(1);
    // First, send headers.
    resp.start(200, 20, "text/plain");
    // Clear buffer so we capture only the chunk.
    g_lwip_sent_buffer.clear();
    
    const char* chunk = "Chunk Data";
    resp.writeChunk(chunk, strlen(chunk));
    
    STRCMP_CONTAINS("Chunk Data", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, SendUnauthorizedSendsProperResponse)
{
    Response resp(1);
    resp.sendUnauthorized();
    
    // Verify that status is set to 401 and correct headers/body are sent.
    STRCMP_CONTAINS("HTTP/1.1 401 Unauthorized", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("Content-Type: application/json", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("{\"error\": \"Unauthorized\"}", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, RenderTemplateReplacesPlaceholders)
{
    Response resp(1);
    std::map<std::string, std::string> context;
    context["name"] = "John";
    context["age"] = "30";
    
    std::string tpl = "Hello, {{name}}. You are {{age}} years old.";
    std::string rendered = resp.renderTemplate(tpl, context);
    
    STRCMP_EQUAL("Hello, John. You are 30 years old.", rendered.c_str());
}

// Additional tests for the remaining Response functions

TEST(HttpResponse, FinishDoesNotSendAdditionalData)
{
    Response resp(1);
    // First, send something so headers and body are written.
    resp.send("Body");
    size_t initialSize = g_lwip_sent_buffer.size();
    resp.finish();
    // Since finish() does nothing, buffer should remain unchanged.
    LONGS_EQUAL(initialSize, g_lwip_sent_buffer.size());
}

TEST(HttpResponse, GetSocketReturnsConstructorValue)
{
    Response resp(42);
    LONGS_EQUAL(42, resp.getSocket());
}

TEST(HttpResponse, SetContentTypeSetsCustomContentType)
{
    Response resp(1);
    // Set a custom content type.
    resp.setContentType("application/json");
    resp.send("Test");
    STRCMP_CONTAINS("Content-Type: application/json", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, SetHeaderAddsCustomHeader)
{
    Response resp(1);
    // setHeader should add a header to the internal headers map.
    resp.setHeader("X-Custom", "foo");
    resp.send("Test");
    STRCMP_CONTAINS("X-Custom: foo", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, RedirectSendsProperRedirectResponse)
{
    Response resp(1);
    // The redirect() helper should set the proper status and Location header.
    resp.redirect("http://example.com", 302);
    STRCMP_CONTAINS("HTTP/1.1 302", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("Location: http://example.com", g_lwip_sent_buffer.c_str());
    // Body should be empty.
    STRCMP_CONTAINS("\r\n\r\n", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, SendNotFoundSends404Response)
{
    Response resp(1);
    resp.sendNotFound();
    STRCMP_CONTAINS("HTTP/1.1 404 Not Found", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("Content-Type: application/json", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("{\"error\": \"Not Found\"}", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, EndServerErrorSends500Response)
{
    Response resp(1);
    resp.endServerError("Oops");
    STRCMP_CONTAINS("HTTP/1.1 500 Internal Server Error", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("Content-Type: application/json", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("{\"error\": \"Oops\"}", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, JsonSendsJsonResponse)
{
    Response resp(1);
    // The json() helper sets Content-Type and sends the JSON string.
    resp.json("{\"ok\":true}");
    STRCMP_CONTAINS("Content-Type: application/json", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("{\"ok\":true}", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, TextSendsPlainTextResponse)
{
    Response resp(1);
    // The text() helper should set Content-Type to text/plain and send the text.
    resp.text("Hello");
    STRCMP_CONTAINS("Content-Type: text/plain", g_lwip_sent_buffer.c_str());
    STRCMP_CONTAINS("Hello", g_lwip_sent_buffer.c_str());
}

TEST(HttpResponse, SetStatusAliasWorks)
{
    Response resp(1);
    // Using setStatus (alias for status) should update the status code.
    resp.setStatus(404);
    resp.send("Test");
    STRCMP_CONTAINS("HTTP/1.1 404", g_lwip_sent_buffer.c_str());
}


