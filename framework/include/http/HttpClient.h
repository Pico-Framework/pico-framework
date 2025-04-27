#pragma once
#include "HttpRequest.h"
#include "HttpResponse.h"

/// @cond INTERNAL
class HttpClient {
public:

    friend class HttpRequest; // Only HttpRequest can use it - httpclient functionality is all provided through HttpRequest

private:

    bool sendRequest(const HttpRequest& request, HttpResponse& response); // common helper

};
/// @endcond