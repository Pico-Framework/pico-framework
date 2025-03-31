/**
 * @file FrameworkView.h
 * @author Ian Archbell
 * @brief Lightweight HTML templating utility for embedded applications.
 * 
 * Part of the PicoFramework application framework.
 * FrameworkView provides a static method to render HTML files with simple
 * {{key}} placeholders, replaced using a key-value context map.
 * 
 * Designed for use in microcontroller-based HTTP servers.
 * 
 * @version 0.1
 * @date 2025-03-31
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #pragma once
 #include <string>
 #include <map>
 
 /**
  * @brief Utility class for rendering static HTML views with variable substitution.
  * 
  * Replaces `{{key}}` placeholders in an HTML template file using the provided context map.
  * 
  * Example:
  * @code
  * std::map<std::string, std::string> ctx = { {"name", "Alice"} };
  * std::string html = FrameworkView::render("/www/index.html", ctx);
  * @endcode
  */
 class FrameworkView {
 public:
     /**
      * @brief Render a template by replacing `{{key}}` with corresponding values.
      * 
      * Loads the file from `templatePath`, then replaces all placeholders
      * matching keys in the context map.
      * 
      * @param templatePath Path to the HTML or text template file.
      * @param context Key-value pairs for substitution.
      * @return Rendered output as a string.
      */
     static std::string render(const std::string& templatePath, const std::map<std::string, std::string>& context);
 };
 