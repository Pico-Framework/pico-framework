/**
 * @file JwtAuthenticator.h
 * @author Ian Archbell
 * @brief Stateless singleton class for creating and validating JWTs using HMAC-SHA256.
 * @version 0.1
 * @date 2025-03-26
 *
 * @license MIT License
 *
 * Provides methods to create, decode, verify, and validate JWTs using base64url encoding
 * and the mbedTLS cryptographic library.
 */

 #ifndef JWT_AUTHENTICATOR_H
 #define JWT_AUTHENTICATOR_H
 
 #include <string>
 
 /**
  * @brief JWT JwtAuthenticator for embedded applications.
  *
  * This class provides helper methods to generate and validate JWT tokens using
  * HMAC-SHA256. It uses a singleton design pattern and is intended to be stateless
  * aside from a secret key.
  */
 class JwtAuthenticator
 {
 public:
     /**
      * @brief Get an instance of the JwtAuthenticator.
      * @return Reference to the JwtAuthenticator instance.
      */
     JwtAuthenticator();
     /**
      * @brief Initialize the JwtAuthenticator with a secret key and expiry time.
      * @param secret Secret key for HMAC-SHA256 signing.
      * @param expirySeconds Expiry time in seconds.
      */
     void init(const std::string &secret, int expirySeconds);
 
     /**
      * @brief Generate a JWT for a given user.
      * @param userId User ID (used as the `sub` claim).
      * @param userName User name (stored in `name` claim).
      * @return A signed JWT string.
      */
     std::string generateJWT(const std::string &userId, const std::string &userName) const;
 
     /**
      * @brief Validate a JWT's signature and optionally its expiration.
      * @param token JWT string.
      * @param validateExpiry Whether to check the `exp` claim.
      * @return true if valid and optionally not expired.
      */
     bool validateJWT(const std::string &token, bool validateExpiry = false) const;
 
     /**
      * @brief Decode a JWT into its components.
      * @param token JWT string.
      * @param header Output decoded header JSON string.
      * @param payload Output decoded payload JSON string.
      * @param signature Output base64url-encoded signature.
      * @return true if decoding was successful.
      */
     bool decodeJWT(const std::string &token, std::string &header, std::string &payload, std::string &signature) const;
 
     /**
      * @brief Check if a JWT is expired based on the `exp` claim.
      * @param token JWT string.
      * @return true if expired, false if valid or error.
      */
     bool isJWTExpired(const std::string &token) const;
 
     /**
      * @brief Check if a decoded JWT payload is expired.
      * @param payload Decoded payload string (JSON).
      * @return true if expired, false otherwise.
      */
     bool isJWTPayloadExpired(const std::string &payload) const;
 
     /**
      * @brief Verify a JWT's signature using HMAC-SHA256.
      * @param encoded_header Base64url-encoded JWT header.
      * @param encoded_payload Base64url-encoded JWT payload.
      * @param signature Base64url-encoded signature to verify against.
      * @return true if the signature is valid.
      */
     bool verifyJWTSignature(const std::string &encoded_header, const std::string &encoded_payload, const std::string &signature) const;
 
 private:
     /**
      * @brief Construct a new JwtAuthenticator object.
      * Secret key is loaded from build config.
      */
 
     // convenience initializer
 
     std::string secretKey;
     std::string expiryTime;
 
     // ------------------------------------------------------------------------
     // Internal Helpers
     // ------------------------------------------------------------------------
 
     /**
      * @brief Encode a string using base64url encoding.
      * @param input Raw input string.
      * @return Base64url-encoded string.
      */
     std::string base64urlEncode(const std::string &input) const;
 
     /**
      * @brief Decode a base64url-encoded string.
      * @param input Encoded input.
      * @param output Decoded output.
      * @return true if decoding was successful.
      */
     bool base64urlDecode(const std::string &input, std::string &output) const;
 
     /**
      * @brief Check if a string is valid base64url.
      * @param str String to check.
      * @return true if string is valid base64url.
      */
     bool isBase64urlEncoded(const std::string &str) const;
 
     /**
      * @brief Convert a byte buffer to a base64url string.
      * @param data Byte array.
      * @param length Length of array.
      * @return Base64url string.
      */
     std::string bytesToBase64url(const unsigned char *data, size_t length) const;
 
     /**
      * @brief Calculate HMAC-SHA256 for a given message.
      * @param message Input string.
      * @return Binary HMAC result.
      */
     std::string hmacSHA256(const std::string &message) const;
 };
 
 #endif // JWT_AUTHENTICATOR_H
 