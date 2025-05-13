/**
 * @file ProgramModel.h
 * @author Ian Archbell
 * @version 0.1
 * @date 2025-05-09
 * @license MIT
 * @copyright Copyright (c) 2025, Ian Archbell
 *
 * /**
 * @brief ZoneModel is responsible for managing the runtime state and configuration of individual watering zones.
 * It handles starting and stopping zones on request (including with a duration),
 * reacts to framework zone events, and persists the zone configuration/state.
 *
 * ZoneModel does NOT know anything about program structure or schedules.
 */

#pragma once
#include "framework/FrameworkModel.h"
#include <string>
#include <map>
#include "RunZone.h"

struct Zone
{
    std::string id;
    std::string name;
    uint8_t gpioPin;
    bool active = false;
    std::string image; 

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Zone, id, name, gpioPin, active, image)
};

class ZoneModel : public FrameworkModel
{
public:
    explicit ZoneModel(const std::string &path);

    // Lifecycle
    bool load();
    bool save();  
    bool save(const std::string &id, const json &data);  // Forwarding method


    // Core functionality
    bool startZone(const std::string &name);
    bool startZone(const std::string &name, uint32_t duration);
    bool stopZone(const std::string &name);
    bool isZoneRunning(const std::string &name) const;
    bool isRunning() const
    {
        return runningZone().has_value();
    }
    
    std::string currentZoneName() const {
        return runningZone().has_value() ? runningZone()->zone : "";
    }
    
    int currentZoneDuration() const {
        return runningZone().has_value() ? runningZone()->duration : 0;
    }

    std::optional<RunZone> runningZone() const {
        return current;
    }

    // Zone update only — no add/remove
    bool updateZone(const std::string& id, const Zone &data);
    bool updateZoneByName(const std::string& name, const Zone &data);

    void rebuildNameIndex();

    const std::vector<Zone>& getAllZones() const { return zones; }

private:
    std::vector<Zone> zones;
    std::unordered_map<std::string, Zone *> nameIndex;
    std::optional<RunZone> current;  // Current running zone

};
