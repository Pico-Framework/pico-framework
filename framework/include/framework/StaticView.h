#pragma once

#include "framework/FrameworkView.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"

#include "framework/FrameworkView.h"

/**
 * @brief StaticView sends a fixed HTML string with no template substitution.
 */
class StaticView : public FrameworkView
{
public:
    explicit StaticView(const char *html) : html_(html) {}

    std::string render(const std::map<std::string, std::string> & = {}) const override
    {
        return html_;
    }

    std::string getContentType() const override
    {
        return "text/html";
    }

private:
    const char *html_;
};
