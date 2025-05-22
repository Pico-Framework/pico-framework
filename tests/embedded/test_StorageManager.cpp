/**
 * @file test_StorageManager.cpp
 * @brief Embedded-safe manual tests for StorageManager (with robust debugging)
 */

#include <CppUTest/TestHarness.h>
#include <vector>
#include <string>
#include <pico/stdlib.h>
#include "storage/StorageManager.h"

#if PICO_HTTP_ENABLE_LITTLEFS
#include "storage/LittleFsStorageManager.h"
static LittleFsStorageManager storageManager;
#else
#include "storage/FatFsStorageManager.h"
static FatFsStorageManager storageManager;
#endif

static StorageManager* storage = &storageManager;

// Helper macro to run and log each test
#define RUN_TEST_FN(name) \
    do { \
        printf("\n=== RUNNING: %s ===\n", #name); \
        try { \
            name(); \
            printf("=== PASSED:  %s ===\n", #name); \
        } catch (const std::exception& e) { \
            printf("!!! FAILED:  %s — Exception: %s\n", #name, e.what()); \
            CHECK_FALSE(true); \
        } catch (...) { \
            printf("!!! FAILED:  %s — Unknown exception\n", #name); \
            CHECK_FALSE(true); \
        } \
    } while(0)

void test_WriteAndReadBackFile()
{
    std::string testFile = "test_file.txt";
    const std::string testContent = "Embedded test content";

    std::vector<uint8_t> data(testContent.begin(), testContent.end());
    CHECK_TRUE(storage->writeFile(testFile, data));

    std::vector<uint8_t> read;
    CHECK_TRUE(storage->readFile(testFile, read));

    std::string result(read.begin(), read.end());
    printf("Result string: '%s' (len=%u)\n", result.c_str(), (unsigned)result.length());

    STRCMP_EQUAL(testContent.c_str(), result.c_str());
}

void test_AppendToFile()
{
    std::string testFile = "test_file.txt";
    const std::string part1 = "abc";
    const std::string part2 = "def";

    CHECK_TRUE(storage->writeFile(testFile,
        reinterpret_cast<const uint8_t*>(part1.data()), part1.size()));
    CHECK_TRUE(storage->appendToFile(testFile,
        reinterpret_cast<const uint8_t*>(part2.data()), part2.size()));

    std::string out;
    bool ok = storage->readFileString(testFile, 0, UINT32_MAX, out);
    printf("readFileString() returned: %s\n", ok ? "true" : "false");
    printf("Result length: %u\n", (unsigned)out.length());

    printf("Byte-wise content dump:\n");
    for (size_t i = 0; i < out.length(); ++i) {
        unsigned char ch = static_cast<unsigned char>(out[i]);
        printf("  [%02zu] = 0x%02x '%c'\n", i, ch,
               (ch >= 32 && ch <= 126) ? ch : '.');
    }

    printf("Safe string: '%.*s'\n", (int)out.length(), out.c_str());

    STRCMP_EQUAL("abcdef", out.c_str());
}



void test_ExistsAndRemove()
{
    std::string testFile = "exists_test.txt";  // unique per test

    CHECK_FALSE(storage->exists(testFile));
    CHECK_TRUE(storage->writeFile(testFile,
        reinterpret_cast<const uint8_t*>("x"), 1));
    CHECK_TRUE(storage->exists(testFile));
    CHECK_TRUE(storage->remove(testFile));
    CHECK_FALSE(storage->exists(testFile));
}

void test_RenameFile()
{
    std::string testFile = "test_file.txt";
    std::string renamedFile = "renamed.txt";

    CHECK_TRUE(storage->writeFile(testFile,
        reinterpret_cast<const uint8_t*>("z"), 1));
    CHECK_TRUE(storage->rename(testFile, renamedFile));
    CHECK_TRUE(storage->exists(renamedFile));
    CHECK_FALSE(storage->exists(testFile));
}

void test_FileSize()
{
    std::string testFile = "test_file.txt";
    std::vector<uint8_t> data = {'a', 'b', 'c'};

    CHECK_TRUE(storage->writeFile(testFile, data));
    LONGS_EQUAL(3, storage->getFileSize(testFile));
}

void test_ListDirectoryIncludesFile()
{
    std::string testFile = "test_file.txt";
    std::vector<uint8_t> data = {'1'};
    CHECK_TRUE(storage->writeFile(testFile, data));

    std::vector<FileInfo> listing;
    CHECK_TRUE(storage->listDirectory("/", listing));

    printf("Directory listing:\n");
    for (const auto& file : listing) {
        printf(" - %s (%u bytes)\n", file.name.c_str(), file.size);
    }

    bool found = false;
    for (const auto& file : listing)
    {
        if (file.name == testFile)
        {
            found = true;
            break;
        }
    }
    CHECK_TRUE(found);
}

void test_StreamFile()
{
    std::string testFile = "test_file.txt";
    std::vector<uint8_t> data(512, 'x');

    CHECK_TRUE(storage->writeFile(testFile, data));

    size_t total = 0;
    CHECK_TRUE(storage->streamFile(testFile, [&](const uint8_t *chunk, size_t len) {
        total += len;
        return true;
    }));

    LONGS_EQUAL(data.size(), total);
}

void test_FormatClearsStorage()
{
    std::string testFile = "test_file.txt";
    CHECK_TRUE(storage->writeFile(testFile,
        reinterpret_cast<const uint8_t*>("y"), 1));
    CHECK_TRUE(storage->exists(testFile));
    CHECK_TRUE(storage->formatStorage());
    CHECK_FALSE(storage->exists(testFile));
}

extern "C" void runStorageManagerTests()
{
    printf("Mounting storage...\n");
    CHECK_TRUE(storage->mount());
    printf("Storage mounted.\n");

    RUN_TEST_FN(test_WriteAndReadBackFile);
    RUN_TEST_FN(test_AppendToFile);
    RUN_TEST_FN(test_ExistsAndRemove);
    RUN_TEST_FN(test_RenameFile);
    RUN_TEST_FN(test_FileSize);
    RUN_TEST_FN(test_ListDirectoryIncludesFile);
    RUN_TEST_FN(test_StreamFile);
    RUN_TEST_FN(test_FormatClearsStorage);

    printf("\nAll embedded StorageManager tests finished.\n");
}
