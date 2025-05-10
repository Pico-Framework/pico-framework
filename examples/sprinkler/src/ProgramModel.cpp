/**
 * @file ProgramModel.cpp
 * @author Ian Archbell
 * @brief Implements sprinkler program storage and access.
 * @version 0.1
 * @date 2025-05-09
 * @license MIT
 * @copyright Copyright (c) 2025, Ian Archbell
 */

#include "ProgramModel.h"
#include "time/PicoTime.h"

ProgramModel::ProgramModel(const std::string &path)
    : FrameworkModel(path) {}

std::vector<SprinklerProgram> &ProgramModel::getPrograms()
{
    return programs;
}

const SprinklerProgram *ProgramModel::get(const std::string &name) const
{
    for (const auto &p : programs)
    {
        if (p.name == name)
        {
            return &p;
        }
    }
    return nullptr;
}

void ProgramModel::saveOrUpdate(const SprinklerProgram &program)
{
    for (auto &p : programs)
    {
        if (p.name == program.name)
        {
            p = program;
            save();
            return;
        }
    }
    programs.push_back(program);
    FrameworkModel::save();
}

void ProgramModel::remove(const std::string &name)
{
    programs.erase(
        std::remove_if(programs.begin(), programs.end(),
                       [&](const SprinklerProgram &p)
                       { return p.name == name; }),
        programs.end());
    save();
}

std::vector<ProgramEvent> ProgramModel::flattenScheduleForDay(Day day) const
{
    std::vector<ProgramEvent> events;
    for (const auto &program : programs)
    {
        if (!(program.days & static_cast<uint8_t>(day)))
            continue;
        for (const auto &z : program.zones)
        {
            ProgramEvent e;
            e.programName = program.name;
            e.zone = z.zone;
            e.start = program.start;
            e.duration = z.duration;
            events.push_back(e);
        }
    }
    // Sort by start time (assuming TimeOfDay supports operator<)
    std::sort(events.begin(), events.end(), [](const ProgramEvent &a, const ProgramEvent &b)
              { return a.start < b.start; });
    return events;
}

const ProgramEvent* ProgramModel::getNextEvent(uint32_t now) const {
    static std::vector<ProgramEvent> events;
    static uint32_t cachedDay = 0xFF;

    TimeOfDay current = PicoTime::toTimeOfDay(now);
    Day today = PicoTime::dayOfWeek(now);

    // Refresh cache if day changed
    if (cachedDay != static_cast<uint8_t>(today)) {
        events = flattenScheduleForDay(today);
        cachedDay = static_cast<uint8_t>(today);
    }

    for (const auto& e : events) {
        if (current < e.start) {
            return &e;
        }
    }
    return nullptr;
}

bool ProgramModel::isEventDue(uint32_t now) const {
    TimeOfDay current = PicoTime::toTimeOfDay(now);
    Day today = PicoTime::dayOfWeek(now);

    auto events = flattenScheduleForDay(today);
    for (const auto& e : events) {
        if (current == e.start) {
            return true;
        }
    }
    return false;
}

bool ProgramModel::load() {
    if (!FrameworkModel::load())
        return false;

    programs.clear();
    for (const auto& entry : all()) {
        if (entry.contains("name") && entry.contains("start") && entry.contains("days") && entry.contains("zones")) {
            programs.push_back(entry.get<SprinklerProgram>());
        } else {
            printf("Invalid program entry in collection\n");
        }
    }

    rebuildNameIndex();
    printf("Loaded %zu programs\n", programs.size());
    return true;
}

bool ProgramModel::save() {
    collection = programs;
    return save();
}

bool ProgramModel::save(const std::string& id, const json& data) {
    return FrameworkModel::save(id, data);
}

void ProgramModel::rebuildNameIndex() {
    nameIndex.clear();
    for (auto& p : programs) {
        nameIndex[p.name] = &p;
    }
}


