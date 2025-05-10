#include "time/TimeOfDay.h"
#include <nlohmann/json.hpp>

inline void to_json(nlohmann::json& j, const TimeOfDay& t) {
    j = TimeOfDay::toString(t); // Convert TimeOfDay to string
}


inline void from_json(const nlohmann::json& j, TimeOfDay& t) {
    t = TimeOfDay::fromString(j.get<std::string>().c_str());
}