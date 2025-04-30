/**
 * @file JsonView.h
 * @brief View for rendering JSON using nlohmann::json, with automatic content type.
 *
 * Part of the PicoFramework view system. Can be used with `HttpResponse::send(view)`
 * to return a structured JSON response.
 *
 * @version 0.3
 * @date 2025-04-30
 * @license MIT License
 */

 #pragma once
 #include "framework/FrameworkView.h"
 #include <nlohmann/json.hpp>
 
 /**
  * @brief A view that renders structured JSON and returns "application/json" content type.
  */
 class JsonView : public FrameworkView {
 public:
     explicit JsonView(const nlohmann::json& payload);
 
     std::string render(const std::map<std::string, std::string>& context = {}) const override;
     std::string getContentType() const override;
 
 private:
     nlohmann::json payload_;
 };
 