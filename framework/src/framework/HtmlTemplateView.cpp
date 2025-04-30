/**
 * @file HtmlTemplateView.cpp
 * @brief Implements HTML template rendering from string or file with {{key}} substitution.
 */

#include "framework/HtmlTemplateView.h"
#include "framework/AppContext.h"
#include "storage/StorageManager.h"

HtmlTemplateView::HtmlTemplateView(const std::string &source, TemplateSource mode)
    : mode_(mode)
{
    if (mode_ == TemplateSource::Inline)
    {
        templateSource_ = source;
    }
    else
    {
        filePath_ = source;
    }
}

std::string HtmlTemplateView::render(const std::map<std::string, std::string> &context) const
{
    std::string tpl;

    if (mode_ == TemplateSource::Inline)
    {
        tpl = templateSource_;
    }
    else
    {
        auto *storage = AppContext::get<StorageManager>();
        std::vector<uint8_t> buf;
        if (storage && storage->readFile(filePath_, buf))
        {
            tpl.assign(buf.begin(), buf.end());
        }
        else
        {
            tpl = "<h1>Template not found</h1>";
        }
    }

    for (const auto &[key, value] : context)
    {
        std::string placeholder = "{{" + key + "}}";
        size_t pos = 0;
        while ((pos = tpl.find(placeholder, pos)) != std::string::npos)
        {
            tpl.replace(pos, placeholder.length(), value);
            pos += value.length();
        }
    }

    return tpl;
}

std::string HtmlTemplateView::getContentType() const
{
    return "text/html";
}
