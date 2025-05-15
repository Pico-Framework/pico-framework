/**
 * @file ProgramModel.h
 * @author Ian Archbell
 * @version 0.1
 * @date 2025-05-09
 * @license MIT
 * @copyright Copyright (c) 2025, Ian Archbell
 *
 * /**
 * @brief ProgramModel owns and manages user-defined watering programs.
 * Each program has a name, start time, set of days, and a list of zone/duration pairs.
 * It supports full CRUD and exposes flattened representations of schedules for execution purposes.
 *
 * ProgramModel does NOT execute zones — it only defines and evaluates schedule information.
 */

#pragma once

#include "framework/FrameworkModel.h"
#include "events/TimerService.h"
#include "time/TimeOfDayJson.h"
#include "RunZone.h"
using ProgramZone = RunZone;

/**
 * @brief Represents a scheduled zone activation as part of a flattened program.
 */
struct ProgramEvent
{
    std::string programName; ///< Source program name
    std::string zone;        ///< Zone to activate
    TimeOfDay start;         ///< Time of day for activation
    uint32_t duration;       ///< Duration in seconds
};

/**
 * @brief Represents a complete sprinkler program with schedule and zone list.
 */
struct SprinklerProgram
{
    std::string name;
    TimeOfDay start;
    DaysOfWeek days;
    std::vector<ProgramZone> zones;

    nlohmann::json toJson() const
    {
        nlohmann::json j;
        j["name"] = name;
        j["start"] = TimeOfDay::toString(start);
        j["days"] = days;
        j["zones"] = nlohmann::json::array();
        for (const auto &z : zones)
        {
            nlohmann::json entry;
            entry["zone"] = z.zone;
            entry["duration"] = z.duration;
            j["zones"].push_back(entry);
        }
        return j;
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SprinklerProgram, name, start, days, zones)
};

/**
 * @brief Manages a list of sprinkler programs with persistence.
 *
 * The programs are loaded from and saved to JSON storage via the base FrameworkModel.
 */
class ProgramModel : public FrameworkModel
{
public:
    /**
     * @brief Construct the model with the given storage path.
     * @param path Path in storage (e.g., "/programs.json")
     */
    explicit ProgramModel(const std::string &path);

    /**
     * @brief Load all programs from persistent storage.
     * @return true if loaded successfully
     */
    bool load();

    /**
     * @brief Save all programs to persistent storage.
     * @return true if saved successfully
     */
    bool save();

    /**
     * @brief Save a json data collection to the model.
     * @return true if saved successfully
     */
    bool save(const std::string& id, const json& data);

    /**
     * @brief Get the full list of programs.
     * @return Reference to the internal list
     */
    std::vector<SprinklerProgram> &getPrograms();

    /**
     * @brief Get a specific program by name.
     * @param name Program name
     * @return Pointer to program if found, or nullptr
     */
    const SprinklerProgram *get(const std::string &name) const;

    /**
     * @brief Add or update a program.
     * @param program The new or updated program definition
     */
    void saveOrUpdate(const SprinklerProgram &program);

    /**
     * @brief Remove a program by name.
     * @param name Program name
     */
    void remove(const std::string &name);

    std::vector<ProgramEvent> flattenScheduleForDay(Day day) const;
    const ProgramEvent *getNextEventForToday(uint32_t now) const;
    const ProgramEvent* getNextEvent(uint32_t now) const;
    bool isEventDue(uint32_t now) const;

    void rebuildNameIndex();

private:
    std::vector<SprinklerProgram> programs;
    std::unordered_map<std::string, SprinklerProgram *> nameIndex;
};