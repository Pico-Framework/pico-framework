#pragma once
#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>

/// @brief Represents a scheduled zone activation.
struct RunZone {
    std::string zone;     ///< Zone name
    uint32_t duration;    ///< Duration in seconds
};

inline void to_json(nlohmann::json &j, const RunZone &z) {
    j = nlohmann::json{{"zone", z.zone}, {"duration", z.duration}};
}

inline void from_json(const nlohmann::json &j, RunZone &z) {
    j.at("zone").get_to(z.zone);
    j.at("duration").get_to(z.duration);
}
