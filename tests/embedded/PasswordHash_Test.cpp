#include "PasswordHasher.h"
#include "CppUTest/TestHarness.h"

TEST_GROUP(PasswordHasher) {};

TEST(PasswordHasher, HashAndVerifyPassword) {
    #ifdef PICO_PLATFORM_RP2350
    PasswordHasher hasher(100000);  // RP2350 hardware accel
    #else
    PasswordHasher hasher(100);    // RP2040 = no hardware accel
    #endif


    std::string password = "hunter2";
    auto salt = PasswordHasher::generateSalt(16);
    auto hash = hasher.hashPassword(password, salt);

    CHECK_EQUAL(hash.size(), PasswordHasher::HASH_LENGTH);
    CHECK_TRUE(hasher.verifyPassword(password, salt, hash));
    CHECK_FALSE(hasher.verifyPassword("wrongpass", salt, hash));
}

TEST(PasswordHasher, Base64RoundTrip) {
    std::vector<uint8_t> original = {0x01, 0x02, 0x03, 0x04};
    auto encoded = PasswordHasher::toBase64(original);
    auto decoded = PasswordHasher::fromBase64(encoded);

    CHECK_EQUAL(original.size(), decoded.size());
    MEMCMP_EQUAL(original.data(), decoded.data(), original.size());
}

TEST(PasswordHasher, HashAndEncodeFormat) {
    #ifdef PICO_PLATFORM_RP2350
    PasswordHasher hasher(100000);  // RP2350 hardware accel
    #else
    PasswordHasher hasher(100);    // RP2040 = no hardware accel
    #endif

    std::string password = "hunter2";
    std::string encoded = hasher.hashAndEncode(password);

    // Ensure format is salt$hash
    auto sep = encoded.find('$');
    CHECK(sep != std::string::npos);
    CHECK_TRUE(hasher.verifyEncoded(password, encoded));
    CHECK_FALSE(hasher.verifyEncoded("wrongpass", encoded));
}

TEST(PasswordHasher, GenerateSaltRandomness) {
    auto s1 = PasswordHasher::generateSalt(16);
    auto s2 = PasswordHasher::generateSalt(16);

    CHECK_EQUAL(s1.size(), 16);
    CHECK_EQUAL(s2.size(), 16);
    CHECK_FALSE(std::equal(s1.begin(), s1.end(), s2.begin()));  // very unlikely to be identical
}

#include "CppUTest/CommandLineTestRunner.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"

#include "CppUTest/TestOutput.h"
#include <cstdio>
