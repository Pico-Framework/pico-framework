#define CPPUTEST_MEM_LEAK_DETECTION_DISABLED 1
#include "CppUTest/TestHarness.h"
#include <vector>\

#include "FatFsStorageManager.h"
#include "JsonService.h"

void writeFileToStorage(FatFsStorageManager& storage, const std::string& path, const std::string& content) {
    std::vector<uint8_t> data(content.begin(), content.end());
    CHECK_TRUE(storage.writeFile(path, data));
}

TEST_GROUP(JsonService)
{
    FatFsStorageManager storage;
    const std::string testFile = "/testfile.txt";
    const std::string renamedFile = "/renamed.txt";

    void setup() {
        printf("Setting up TEST_GROUP\n");
        storage.mount();
        storage.remove(testFile); // cleanup in case of leftover
        storage.remove(renamedFile);
    }

    void teardown() {
        storage.remove(testFile);
        storage.remove(renamedFile);
        storage.unmount();
    }
};

TEST(JsonService, Json_CanLoadValidJson)
{
    const std::string jsonContent = R"({"foo": 123, "bar": "baz"})";
    writeFileToStorage(storage, testFile, jsonContent);

    JsonService json(&storage);
    CHECK_TRUE(json.load(testFile));
    CHECK_EQUAL(123, json.data()["foo"].get<int>());
    STRCMP_EQUAL("baz", json.data()["bar"].get<std::string>().c_str());
}

TEST(JsonService, Json_CanSaveAndReloadJson)
{
    JsonService json(&storage);
    json.data()["hello"] = "world";
    CHECK_TRUE(json.save(testFile));

    JsonService jsonReload(&storage);
    CHECK_TRUE(jsonReload.load(testFile));
    STRCMP_EQUAL("world", jsonReload.data()["hello"].get<std::string>().c_str());
}

TEST(JsonService, Json_LoadFailsIfFileDoesNotExist)
{
    JsonService json(&storage);
    CHECK_FALSE(json.load("/nonexistent.json"));
}

TEST(JsonService, Json_LoadFailsOnInvalidJson)
{
    writeFileToStorage(storage, testFile, R"({ invalid json } )");
    JsonService json(&storage);
    CHECK_FALSE(json.load(testFile));
}

TEST(JsonService, Json_EmptyFileGivesEmptyObject)
{
    writeFileToStorage(storage, testFile, "");
    JsonService json(&storage);
    CHECK_TRUE(json.load(testFile));
    CHECK_TRUE(json.data().is_object());
    CHECK_TRUE(json.data().empty());
}

TEST(JsonService, Json_CanOverwriteExistingJson)
{
    // Step 1: Save initial object
    {
        JsonService json(&storage);
        json.data()["value"] = 1;
        CHECK_TRUE(json.save(testFile));
    }

    // Step 2: Overwrite it
    {
        JsonService json(&storage);
        json.data()["value"] = 999;
        CHECK_TRUE(json.save(testFile));
    }

    // Step 3: Reload and verify
    {
        JsonService json(&storage);
        CHECK_TRUE(json.load(testFile));
        CHECK_EQUAL(999, json.data()["value"].get<int>());
    }
}

#include "CppUTest/CommandLineTestRunner.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"

#include "CppUTest/TestOutput.h"
#include <cstdio>

