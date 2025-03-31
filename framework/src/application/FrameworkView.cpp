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
 * void handleHome(Request& req, Response& res) {
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

 #include "FrameworkView.h"
 #include <fstream>
 #include <sstream>
 
 /// @brief Loads an entire file into a string (helper for render()).
 static std::string loadFile(const std::string& path) {
     std::ifstream file(path);
     std::stringstream buffer;
     buffer << file.rdbuf();
     return buffer.str();
 }
 
 /// @copydoc FrameworkView::render
 std::string FrameworkView::render(const std::string& templatePath, const std::map<std::string, std::string>& context) {
     std::string tpl = loadFile(templatePath);
     for (const auto& [key, value] : context) {
         std::string placeholder = "{{" + key + "}}";
         size_t pos = 0;
         while ((pos = tpl.find(placeholder, pos)) != std::string::npos) {
             tpl.replace(pos, placeholder.length(), value);
             pos += value.length();
         }
     }
     return tpl;
 }
 

 
