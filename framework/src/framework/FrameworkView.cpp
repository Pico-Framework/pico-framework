/**
 * @file FrameworkView.cpp
 * @author Ian Archbell
 * @brief Implementation of simple embedded HTML template renderer.
 *
 * Part of the PicoFramework application framework.
 * This module implements the FrameworkView class, which provides a static
 * method to render HTML templates with variable substitution.
 * It replaces `{{key}}` placeholders in the HTML file with corresponding
 * values from a provided context map.
 *
 * Loads HTML files and performs basic {{key}} substitution from a string map.
 * Useful for serving dynamic content on embedded HTTP servers.
 *
 * Example usage:
 *
 * @code
 * void handleHome(HttpRequest& req, HttpResponse& res) {
 *    std::map<std::string, std::string> context = {
 *           {"name", "Alice"},
 *           {"time", getCurrentTimeString()}
 *    };
 *    std::string html = FrameworkView::render("/www/index.html", context);
 *    res.send(html, "text/html");
 * }
 * @endcode
 *
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "framework/FrameworkView.h"
#include <fstream>
#include <sstream>

/// @brief Loads an entire file into a string (helper for render()).
static std::string loadFile(const std::string &path)
{
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

/// @copydoc FrameworkView::render
// res.send(View::render("page.html", {{"title", "Hi"}}))
std::string FrameworkView::render(const std::string &templatePath, const std::map<std::string, std::string> &context)
{
    std::string tpl = loadFile(templatePath);
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

std::string FrameworkView::render(const std::string& path, const nlohmann::json& jsonContext) {
    std::map<std::string, std::string> context;

    for (auto it = jsonContext.begin(); it != jsonContext.end(); ++it) {
        if (it.value().is_string()) {
            context[it.key()] = it.value();
        } else {
            context[it.key()] = it.value().dump();  // Serialize numbers, arrays, bools etc.
        }
    }

    return render(path, context);  // Call existing version
}


/**
 * @brief Renders a JSON object as a pretty-printed string.
 * e.g res.send(FrameworkView::renderJson(myModel.findAsJson("abc")), "application/json");
 */

std::string FrameworkView::renderJson(const nlohmann::json& json) {
    return json.dump(2); // Pretty print with indent = 2
}
