/**
 * @file JwtAuthenticator.cpp
 * @author Ian Archbell
 * @brief Implementation of Authenticator for JWT creation and validation.
 * @version 0.1
 * @date 2025-03-26
 * 
 * @license MIT License
 * 
 */

 #include "JwtAuthenticator.h"

 #include <iostream>
 #include <chrono>
 #include <mbedtls/md.h>
 #include <mbedtls/base64.h>
 #include <mbedtls/error.h>
 #include <nlohmann/json.hpp>
 
 using json = nlohmann::json;
 
 #ifndef MBEDTLS_SHA256_DIGEST_LENGTH
 #define MBEDTLS_SHA256_DIGEST_LENGTH 32
 #endif
 
 /// @copydoc Authenticator::Authenticator
 Authenticator::Authenticator()
 {
     secretKey = JWT_SECRET;
 }
 
 /// @copydoc Authenticator::base64urlEncode
 std::string Authenticator::base64urlEncode(const std::string& input) const
 {
     size_t encoded_len = ((input.length() + 2) / 3) * 4 + 1;
     char* encoded_data = new char[encoded_len];
 
     size_t len = 0;
     int result = mbedtls_base64_encode(reinterpret_cast<unsigned char*>(encoded_data), encoded_len, &len,
                                        reinterpret_cast<const unsigned char*>(input.c_str()), input.length());
 
     if (result != 0) {
         std::cerr << "Base64 encoding failed with error code: " << result << std::endl;
         delete[] encoded_data;
         return "";
     }
 
     std::string base64_str(encoded_data, len);
     std::string base64url_str(base64_str);
     std::replace(base64url_str.begin(), base64url_str.end(), '+', '-');
     std::replace(base64url_str.begin(), base64url_str.end(), '/', '_');
     base64url_str.erase(std::remove(base64url_str.begin(), base64url_str.end(), '='), base64url_str.end());
 
     delete[] encoded_data;
     return base64url_str;
 }
 
 /// @copydoc Authenticator::base64urlDecode
 bool Authenticator::base64urlDecode(const std::string& input, std::string& output) const
 {
     std::string base64_input = input;
     std::replace(base64_input.begin(), base64_input.end(), '-', '+');
     std::replace(base64_input.begin(), base64_input.end(), '_', '/');
     while (base64_input.size() % 4 != 0) {
         base64_input += "=";
     }
 
     for (size_t i = 0; i < base64_input.size(); ++i) {
         if (static_cast<unsigned char>(base64_input[i]) > 127) {
             std::cout << "Invalid character found in base64: " << (int)base64_input[i] << std::endl;
             return false;
         }
     }
 
     size_t decoded_len = base64_input.size() * 3 / 4 + 4;
     unsigned char* decoded_data = new unsigned char[decoded_len];
 
     int ret = mbedtls_base64_decode(decoded_data, decoded_len, &decoded_len,
                                     (const unsigned char*)base64_input.c_str(), base64_input.size());
 
     if (ret != 0) {
         char error_msg[100];
         mbedtls_strerror(ret, error_msg, sizeof(error_msg));
         std::cerr << "Base64 decoding failed: " << error_msg << std::endl;
         delete[] decoded_data;
         return false;
     }
 
     output.assign(reinterpret_cast<char*>(decoded_data), decoded_len);
     delete[] decoded_data;
     return true;
 }
 
 /// @copydoc Authenticator::isBase64urlEncoded
 bool Authenticator::isBase64urlEncoded(const std::string& str) const
 {
     static const std::string base64url_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
 
     if (str.length() % 4 != 0) {
         return false;
     }
 
     for (char c : str) {
         if (base64url_chars.find(c) == std::string::npos) {
             return false;
         }
     }
 
     return true;
 }
 
 /// @copydoc Authenticator::hmacSHA256
 std::string Authenticator::hmacSHA256(const std::string& message) const
 {
     unsigned char hmac_output[MBEDTLS_SHA256_DIGEST_LENGTH];
     mbedtls_md_context_t ctx;
 
     mbedtls_md_init(&ctx);
     const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
     mbedtls_md_setup(&ctx, md_info, 1);
     mbedtls_md_hmac_starts(&ctx, reinterpret_cast<const unsigned char*>(secretKey.c_str()), secretKey.size());
     mbedtls_md_hmac_update(&ctx, reinterpret_cast<const unsigned char*>(message.c_str()), message.size());
     mbedtls_md_hmac_finish(&ctx, hmac_output);
     mbedtls_md_free(&ctx);
 
     return std::string(reinterpret_cast<char*>(hmac_output), MBEDTLS_SHA256_DIGEST_LENGTH);
 }
 
 /// @copydoc Authenticator::generateJWT
 std::string Authenticator::generateJWT(const std::string& userId, const std::string& userName) const
 {
     json header = { {"alg", "HS256"}, {"typ", "JWT"} };
     json payload = {
         {"sub", userId},
         {"name", userName},
         {"iat", std::time(0)},
         {"exp", std::time(0) + 3600}
     };
 
     std::string encodedHeader = base64urlEncode(header.dump());
     std::string encodedPayload = base64urlEncode(payload.dump());
     std::string message = encodedHeader + "." + encodedPayload;
     std::string signature = base64urlEncode(hmacSHA256(message));
 
     return encodedHeader + "." + encodedPayload + "." + signature;
 }
 
 /// @copydoc Authenticator::decodeJWT
 bool Authenticator::decodeJWT(const std::string& token, std::string& header, std::string& payload, std::string& signature) const
 {
     size_t first_dot = token.find('.');
     size_t second_dot = token.find('.', first_dot + 1);
     if (first_dot == std::string::npos || second_dot == std::string::npos) {
         return false;
     }
 
     header = token.substr(0, first_dot);
     payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
     signature = token.substr(second_dot + 1);
 
     if (isBase64urlEncoded(header)) {
         std::string decoded;
         if (!base64urlDecode(header, decoded)) return false;
         header = decoded;
     }
 
     if (isBase64urlEncoded(payload)) {
         std::string decoded;
         if (!base64urlDecode(payload, decoded)) return false;
         payload = decoded;
     }
 
     return true;
 }
 
 /// @copydoc Authenticator::bytesToBase64url
 std::string Authenticator::bytesToBase64url(const unsigned char* data, size_t length) const
 {
     size_t output_len = length * 4 / 3 + 4;
     char* base64_output = new char[output_len];
     size_t written_len = 0;
 
     mbedtls_base64_encode((unsigned char*)base64_output, output_len, &written_len, data, length);
 
     std::string base64_result(base64_output, written_len);
     std::replace(base64_result.begin(), base64_result.end(), '+', '-');
     std::replace(base64_result.begin(), base64_result.end(), '/', '_');
     base64_result.erase(std::remove(base64_result.end() - 1, base64_result.end(), '='), base64_result.end());
 
     delete[] base64_output;
     return base64_result;
 }
 
 /// @copydoc Authenticator::verifyJWTSignature
 bool Authenticator::verifyJWTSignature(const std::string& encoded_header, const std::string& encoded_payload, const std::string& signature) const
 {
     std::string header_payload = encoded_header + "." + encoded_payload;
 
     mbedtls_md_context_t ctx;
     mbedtls_md_init(&ctx);
     const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
     mbedtls_md_setup(&ctx, md_info, 1);
     mbedtls_md_hmac_starts(&ctx, (const unsigned char*)secretKey.c_str(), secretKey.length());
     mbedtls_md_hmac_update(&ctx, (const unsigned char*)header_payload.c_str(), header_payload.length());
 
     unsigned char hmac_output[MBEDTLS_SHA256_DIGEST_LENGTH];
     mbedtls_md_hmac_finish(&ctx, hmac_output);
     mbedtls_md_free(&ctx);
 
     std::string computed_signature = bytesToBase64url(hmac_output, MBEDTLS_SHA256_DIGEST_LENGTH);
     return computed_signature == signature;
 }
 
 /// @copydoc Authenticator::isJWTPayloadExpired
 bool Authenticator::isJWTPayloadExpired(const std::string& payload) const
 {
     auto parsed = json::parse(payload, nullptr, false);
     if (parsed.is_discarded() || !parsed.contains("exp") || !parsed["exp"].is_number_integer()) {
         std::cerr << "Invalid or missing 'exp' in JWT payload." << std::endl;
         return false;
     }
 
     long long exp_timestamp = parsed["exp"].get<long long>();
     if (exp_timestamp <= 0) return false;
 
     auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
     return now >= exp_timestamp;
 }
 
 /// @copydoc Authenticator::isJWTExpired
 bool Authenticator::isJWTExpired(const std::string& token) const
 {
     size_t first_dot = token.find('.');
     size_t second_dot = token.find('.', first_dot + 1);
     if (first_dot == std::string::npos || second_dot == std::string::npos) return false;
 
     std::string encoded_payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
     std::string decoded_payload;
 
     if (!base64urlDecode(encoded_payload, decoded_payload)) return false;
     return isJWTPayloadExpired(decoded_payload);
 }
 
 /// @copydoc Authenticator::validateJWT
 bool Authenticator::validateJWT(const std::string& token, bool validateExpiry) const
 {
     size_t first_dot = token.find('.');
     size_t second_dot = token.find('.', first_dot + 1);
     if (first_dot == std::string::npos || second_dot == std::string::npos) return false;
 
     std::string encoded_header = token.substr(0, first_dot);
     std::string encoded_payload = token.substr(first_dot + 1, second_dot - first_dot - 1);
     std::string signature = token.substr(second_dot + 1);
 
     std::string decoded_header, decoded_payload;
     if (!base64urlDecode(encoded_header, decoded_header) ||
         !base64urlDecode(encoded_payload, decoded_payload)) {
         return false;
     }
 
     if (validateExpiry && isJWTPayloadExpired(decoded_payload)) {
         return false;
     }
 
     return verifyJWTSignature(encoded_header, encoded_payload, signature);
 }
 