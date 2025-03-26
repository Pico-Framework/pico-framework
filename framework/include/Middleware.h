/**
 * @file Middleware.h
 * @author Ian Archbell
 * @brief 
 * @version 0.1
 * @date 2025-03-26
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef MIDDLEWARE_HPP
#define MIDDLEWARE_HPP
#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "JwtAuthenticator.h"
#include <functional>
#include <vector>

using Middleware = std::function<bool(Request&, Response&, const std::vector<std::string>&)>;

extern Middleware authMiddleware;  // Declare middleware

extern Middleware loggingMiddleware;  // Declare middleware

#endif // MIDDLEWARE_HPP