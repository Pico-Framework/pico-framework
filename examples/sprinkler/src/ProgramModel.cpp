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
    clearScheduleCache();  // clear cache first

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
    save();  // 
}


void ProgramModel::remove(const std::string &name)
{
    programs.erase(
        std::remove_if(programs.begin(), programs.end(),
                       [&](const SprinklerProgram &p)
                       { return p.name == name; }),
        programs.end());
    clearScheduleCache();
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

const ProgramEvent* ProgramModel::getNextEventForToday(uint32_t now) const {
    TimeOfDay current = PicoTime::toTimeOfDay(now);
    Day today = PicoTime::dayOfWeek(now);

    if (cachedToday != static_cast<uint8_t>(today)) {
        cachedTodayEvents = flattenScheduleForDay(today);
        cachedToday = static_cast<uint8_t>(today);
    }

    for (const auto& e : cachedTodayEvents) {
        if (current < e.start) {
            return &e;
        }
    }
    return nullptr;
}


const ProgramEvent* ProgramModel::getNextEvent(uint32_t now) const {

    if (cachedEvents.empty() || now < lastGenerated || now > lastGenerated + 60) {
        cachedEvents.clear();

        for (int i = 0; i < 7; ++i) {
            Day day = static_cast<Day>((static_cast<int>(PicoTime::dayOfWeek(now)) + i) % 7);

            auto daily = flattenScheduleForDay(day);

            for (const auto& e : daily) {
                struct tm t = PicoTime::nowTm();
                t.tm_hour = e.start.hour;
                t.tm_min = e.start.minute;
                t.tm_sec = 0;
                t.tm_mday += i;
                time_t when = mktime(&t);
                  
                if (when > (time_t)now) {
                    ProgramEvent future = e;
                    future.when = when;  // attach absolute timestamp
                    cachedEvents.push_back(future);
                }
            }
        }

        std::sort(cachedEvents.begin(), cachedEvents.end(), [](const ProgramEvent& a, const ProgramEvent& b) {
            return a.when < b.when;
        });

        lastGenerated = now;
    }

    if (!cachedEvents.empty()) {
        const auto& first = cachedEvents[0];
               first.programName.c_str(),
               first.zone.c_str(),
               first.start.hour, first.start.minute;
    } else {
        printf("[ProgramModel] No future events cached\n");
    }

    return cachedEvents.empty() ? nullptr : &cachedEvents[0];
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

    for (const auto& entry : FrameworkModel::all()) {
        if (!entry.is_object()) {
            printf("ProgramModel: Skipping non-object entry\n");
            continue;
        }

        if (entry.contains("name") &&
            entry.contains("start") &&
            entry.contains("days") &&
            entry.contains("zones")) {

            SprinklerProgram p;
            p.name = entry.value("name", "");
            p.start = entry.at("start").get<TimeOfDay>();  // uses yfrom_json() to parse "HH:MM" into a TimeOfDay.
            p.days = entry.value("days", 0);

            const auto& zoneArray = entry["zones"];
            if (!zoneArray.is_array()) {
                printf("ProgramModel: Skipping entry with invalid zones array\n");
                continue;
            }

            for (const auto& z : zoneArray) {
                if (z.contains("zone") && z.contains("duration")) {
                    RunZone rz;
                    rz.zone = z.value("zone", "");
                    rz.duration = z.value("duration", 0);
                    p.zones.push_back(rz);
                }
            }

            programs.push_back(p);
            collection.push_back(p);
        } else {
            printf("Invalid program entry in collection\n");
        }
    }

    rebuildNameIndex();
    printf("[ProgramModel] Loaded %zu programs\n", programs.size());
    return true;
}

bool ProgramModel::save() {
    collection.clear();
    for (const auto& program : programs) {
        collection.push_back(program);  // relies on to_json for SprinklerProgram
    }
    return FrameworkModel::save();
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

void ProgramModel::clearScheduleCache() {
    cachedEvents.clear();
    lastGenerated = 0;
    cachedTodayEvents.clear();
    cachedToday = 0xFF;
}
