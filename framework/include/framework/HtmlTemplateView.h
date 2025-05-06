/**
 * @file HtmlTemplateView.h
 * @author Ian Archbell
 * @brief HTML template rendering view, supporting both inline and file-based templates.
 *
 * Supports simple {{key}} substitution for embedded or file-loaded HTML templates.
 * Compatible with the FrameworkView interface and HttpResponse::send(view, context).
 *
 * @version 0.3
 * @date 2025-04-30
 * @license MIT License
 */

 #pragma once
 #include "framework/FrameworkView.h"
 #include <string>
 #include "http/HttpRequest.h"
 #include "http/HttpResponse.h"
 
 enum class TemplateSource {
     Inline,
     FromFile
 };
 
 class HtmlTemplateView : public FrameworkView {
 public:
     HtmlTemplateView(const std::string& source, TemplateSource mode = TemplateSource::Inline);
 
     std::string render(const std::map<std::string, std::string>& context = {}) const override;

     void render(HttpRequest& req, HttpResponse& res) {
        res.setContentType("text/html");
        res.send(render());
    }
    
     std::string getContentType() const override;
 
 private:
     std::string templateSource_;   // Used if mode == Inline
     std::string filePath_;         // Used if mode == FromFile
     TemplateSource mode_;
 };
 