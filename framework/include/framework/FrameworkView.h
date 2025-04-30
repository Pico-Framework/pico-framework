/**
 * @file FrameworkView.h
 * @brief Abstract base class for all views in the PicoFramework.
 *
 * Supports dynamic rendering and MIME type negotiation. View subclasses
 * can define their own rendering logic, content type, and headers.
 *
 * @version 0.3
 * @date 2025-04-30
 * @license MIT License
 */

 #pragma once
 #include <string>
 #include <map>
 #include <nlohmann/json.hpp>
 
 // Forward declaration
 class HttpResponse;
 
 class FrameworkView {
 public:
     virtual ~FrameworkView() = default;
 
     /**
      * @brief Render the view body. Optional context (used by templates).
      */
     virtual std::string render(const std::map<std::string, std::string>& context = {}) const = 0;
 
     /**
      * @brief Return the MIME content type for this view.
      */
     virtual std::string getContentType() const = 0;
 
     /**
      * @brief Optional hook to set response headers (e.g., Content-Disposition).
      */
     virtual void applyHeaders(HttpResponse& response) const {}
 
     /**
      * @brief Optional JSON context hook (used by JsonView or convenience).
      */
     virtual std::string render(const nlohmann::json& context) const {
         return render();  // fallback ignores json
     }
 };
 