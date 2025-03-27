#include "JsonService.h"
#include <cstdio>  // for printf


// json loaded = {
//     {"device", {{"enabled", true}}}
// };

// json defaults = {
//     {"network", {{"ssid", "wifi"}, {"password", "1234"}}},
//     {"device", {{"enabled", false}, {"name", "sprinkler"}}}
// };

// loaded = mergeDefaults(loaded, defaults);

// std::cout << loaded.dump(2) << std::endl;

// {
//     "device": {
//       "enabled": true,
//       "name": "sprinkler"
//     },
//     "network": {
//       "password": "1234",
//       "ssid": "wifi"
//     }
//}


nlohmann::json mergeDefaults(const nlohmann::json& target, const nlohmann::json& defaults) {
    nlohmann::json result = target;

    for (auto it = defaults.begin(); it != defaults.end(); ++it) {
        const std::string& key = it.key();

        if (!result.contains(key)) {
            result[key] = it.value();
        } else if (result[key].is_object() && it.value().is_object()) {
            result[key] = mergeDefaults(result[key], it.value());
        }
    }

    return result;
}

JsonService::JsonService(StorageManager* storage)
    : storage(storage) {}

bool JsonService::load(const std::string& path) {
    std::vector<uint8_t> buffer;
    if (!storage || !storage->readFile(path, buffer)) {
        printf("JsonService: Failed to read %s\n", path.c_str());
        return false;
    }

    std::string content(buffer.begin(), buffer.end());

    nlohmann::json parsed = nlohmann::json::parse(content, nullptr, false);  // No exceptions
    if (parsed.is_discarded()) {
        printf("JsonService: Failed to parse JSON from %s\n", path.c_str());
        return false;
    }

    jsonData = parsed;
    return true;
}

bool JsonService::save(const std::string& path) {
    std::string content = jsonData.dump(4);
    std::vector<uint8_t> data(content.begin(), content.end());

    if (!storage || !storage->writeFile(path, data)) {
        printf("JsonService: Failed to write %s\n", path.c_str());
        return false;
    }

    return true;
}

nlohmann::json& JsonService::data() {
    return jsonData;
}

const nlohmann::json& JsonService::data() const {
    return jsonData;
}

// auto* storage = AppContext::instance().getStorageManager();
// JsonService config(storage);

// if (config.load("/config.json")) {
//     config.data()["device"]["enabled"] = true;
//     config.save("/config.json");
// }


