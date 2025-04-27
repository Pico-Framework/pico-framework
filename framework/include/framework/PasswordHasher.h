#pragma once

#include <string>
#include <vector>
#include <cstdint>  // For uint8_t

class PasswordHasher {
public:

    static constexpr int DEFAULT_ITERATIONS = 1000;
    static constexpr size_t HASH_LENGTH = 32;

    /**
     * @brief Constructs a PasswordHasher with the specified number of iterations.
     * 
     * @param iterations Number of iterations for the hashing algorithm. Default is 1,000.
     */
    PasswordHasher(int iterations = DEFAULT_ITERATIONS);

    /**
     * @brief Hashes a password with the given salt.
     * 
     * @param password The password to hash.
     * @param salt The salt to use for hashing.
     * @return A vector containing the hashed password.
     */
    std::vector<uint8_t> hashPassword(const std::string& password, const std::vector<uint8_t>& salt);

    /**
     * @brief Verifies a password against an expected hash using the provided salt.
     * 
     * @param password The password to verify.
     * @param salt The salt used for hashing.
     * @param expectedHash The expected hash to compare against.
     * @return true if the password matches the expected hash, false otherwise.
     */
    bool verifyPassword(const std::string& password, const std::vector<uint8_t>& salt, const std::vector<uint8_t>& expectedHash);

    /**
     * @brief Hashes a password and encodes it in Base64 format.
     * 
     * @param password The password to hash.
     * @return A Base64 encoded string of the hashed password.
     */
    std::string hashPasswordBase64(const std::string& password, const std::vector<uint8_t>& salt);

    /**
     * @brief Verifies a password against an expected Base64 encoded hash using the provided salt.
     * 
     * @param password The password to verify.
     * @param salt The salt used for hashing.
     * @param expectedBase64Hash The expected Base64 encoded hash to compare against.
     * @return true if the password matches the expected Base64 hash, false otherwise.
     */
    bool verifyPasswordBase64(const std::string& password, const std::vector<uint8_t>& salt, const std::string& expectedBase64Hash);

    /**
     * @brief Hashes a password and encodes it with a salt in Base64 format.
     * 
     * @param password The password to hash.
     * @return A Base64 encoded string containing the salt and the hashed password.
     * The format is "salt:hash", where both salt and hash are Base64 encoded.
     * @note The salt is generated internally and prepended to the hash.
     * The salt is always 16 bytes long, and the hash is 32 bytes long.
     * The resulting string will be 44 characters long (32 for the hash, 16 for the salt, and 1 for the colon).
     * @warning The salt is not stored separately; it is part of the encoded string.
     * This means that the salt must be extracted from the string when verifying the password.
     * @example
     * std::string encoded = hasher.hashAndEncode("my_password");
     * encoded will be in the format "salt:hash" where both salt and hash are Base64 encoded.
     * @note This method is useful for storing passwords securely in a single string format.   
     * 
     */
    std::string hashAndEncode(const std::string& password);

    /**
     * @brief Verifies a password against an encoded salt and hash.
     * 
     * @param password The password to verify.
     * @param combinedSaltAndHash The Base64 encoded string containing the salt and hash in the format "salt:hash".
     * @return true if the password matches the encoded hash, false otherwise.
     * @note The salt is extracted from the encoded string, and the hash is verified against the provided password.
     * The salt is always 16 bytes long, and the hash is 32 bytes long.
     * The expected format of combinedSaltAndHash is "salt:hash", where both salt and hash are Base64 encoded.
     * @example
     * bool isValid = hasher.verifyEncoded("my_password", "c29tZV9zYWx0Omhhc2g=");
     * isValid will be true if the password matches the hash, false otherwise.
     * @warning The combinedSaltAndHash must be in the correct format; otherwise, verification will fail.
     */
    bool verifyEncoded(const std::string& password, const std::string& combinedSaltAndHash);

    /**
     * @brief Converts a vector of bytes to a Base64 encoded string.
     * 
     * @param data The vector of bytes to encode.
     * @return A Base64 encoded string representation of the input data.
     * @note This method is useful for encoding binary data into a text format that can be easily stored or transmitted.
     * @example
     * std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
     * std::string base64 = PasswordHasher::toBase64(data);
     * base64 will contain the Base64 encoded string of the input data.
     */
    static std::string toBase64(const std::vector<uint8_t>& data);

    /**
     * @brief Converts a Base64 encoded string back to a vector of bytes.
     * 
     * @param base64 The Base64 encoded string to decode.
     * @return A vector of bytes representing the decoded data.
     * @note This method is useful for decoding Base64 strings back into their original binary form.
     * @example
     * std::string base64 = "c29tZV9kYXRh";
     * std::vector<uint8_t> data = PasswordHasher::fromBase64(base64);
     * data will contain the decoded bytes from the Base64 string.
     */
    static std::vector<uint8_t> fromBase64(const std::string& base64);

    /**
     * @brief Generates a random salt of the specified length.
     * 
     * @param length The length of the salt to generate in bytes. Default is 16 bytes.
     * @return A vector containing the generated salt.
     * @note The salt is used to add randomness to the password hashing process.
     * It should be unique for each password to ensure security.
     * @example
     * std::vector<uint8_t> salt = PasswordHasher::generateSalt(16);
     */
    static std::vector<uint8_t> generateSalt(size_t length);

private:
    int iterations;
};
