#pragma once
#include "HttpRequest.h"
#include "HttpResponse.h"

/// @cond INTERNAL
class HttpClient {
public:

    friend class HttpRequest; // Only HttpRequest can use it - httpclient functionality is all provided through HttpRequest
    
    // we may need to add sendChunked etc. these helper methods will be added here

private:
    bool sendRequest(const HttpRequest& request, HttpResponse& response); // common helper

};
/// @endcond