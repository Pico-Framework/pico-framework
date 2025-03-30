/**
 * @file JsonRequestHelper.h
 * @author Ian Archbell
 * @brief Helper functions to extract fields from JSON request bodies.
 * @version 0.1
 * @date 2025-03-26
 * 
 * @license MIT License
 */

 #pragma once

 #include <string>
 #include <nlohmann/json.hpp>
 
 using json = nlohmann::json;
 
 class Request;
 
 /**
  * @brief Utility class for working with JSON content in HTTP requests.
  */
 class JsonRequestHelper
 {
 public:
     /**
      * @brief Extract a nested JSON value using a dot-separated path (e.g., "user.name").
      * @param req The incoming request object.
      * @param path Dot-separated path to the value.
      * @return Extracted JSON value or null.
      */
     static json getJsonValue(const Request& req, const std::string& path);
 
     /**
      * @brief Check if a JSON field exists.
      * @param req The incoming request.
      * @param key Dot-separated key path.
      * @return true if the field exists and is not null.
      */
     static bool hasField(const Request& req, const std::string& key);
 
     /**
      * @brief Get a string value from the request body.
      * @param req Request containing JSON body.
      * @param key Dot-separated path to the string field.
      * @return The string value or empty string if not found.
      */
     static std::string getString(const Request& req, const std::string& key);
 
     /**
      * @brief Get an integer value from the request body.
      * @param req Request containing JSON body.
      * @param key Dot-separated path to the integer field.
      * @param def Default value if key is missing or not an integer.
      * @return The integer value or default.
      */
     static int getInt(const Request& req, const std::string& key, int def = 0);
 
     /**
      * @brief Get a double value from the request body.
      * @param req Request containing JSON body.
      * @param key Dot-separated path to the double field.
      * @param def Default value if key is missing or not numeric.
      * @return The double value or default.
      */
     static double getDouble(const Request& req, const std::string& key, double def = 0.0);
 
     /**
      * @brief Get a boolean value from the request body.
      * @param req Request containing JSON body.
      * @param key Dot-separated path to the boolean field.
      * @param def Default value if key is missing or not boolean.
      * @return The boolean value or default.
      */
     static bool getBool(const Request& req, const std::string& key, bool def = false);
 
     /**
      * @brief Parse and return the full JSON body from the request.
      * @param req The HTTP request object.
      * @return Parsed JSON object or empty `{}` if parsing fails.
      */
     static json getFullJson(const Request& req);
 
     /**
      * @brief Get a JSON array from the request body.
      * @param req Request with JSON body.
      * @param key Dot-separated path to the array.
      * @return The array if found and valid, or an empty array.
      */
     static json getArray(const Request& req, const std::string& key);
 
     /**
      * @brief Get a JSON object from the request body.
      * @param req Request with JSON body.
      * @param key Dot-separated path to the object.
      * @return The object if found and valid, or an empty object.
      */
     static json getObject(const Request& req, const std::string& key);
 
 private:
     /**
      * @brief Parse the raw body into a JSON object.
      * @param req Request containing JSON body.
      * @return Parsed JSON object, or `{}` on failure.
      */
     static json parseJsonBody(const Request& req);
 };

 /// @brief Shorthand macro for JsonRequestHelper static methods.
#define JSON JsonRequestHelper
 