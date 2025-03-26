#ifndef MIDDLEWARE_HPP
#define MIDDLEWARE_HPP

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "JwtAuthenticator.h"
#include <functional>
#include <vector>

using Middleware = std::function<bool(Request&, Response&, const std::vector<std::string>&)>;

extern Middleware authMiddleware;  // Declare middleware

extern Middleware loggingMiddleware;  // Declare middleware

#endif // MIDDLEWARE_HPP