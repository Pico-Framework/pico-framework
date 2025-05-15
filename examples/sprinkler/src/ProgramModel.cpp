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

const ProgramEvent* ProgramModel::getNextEventForToday(uint32_t now) const {
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

const ProgramEvent* ProgramModel::getNextEvent(uint32_t now) const {
    static std::vector<ProgramEvent> cachedEvents;
    static uint32_t lastGenerated = 0;
    static time_t lastNow = 0;

    if (cachedEvents.empty() || now < lastNow || now > lastNow + 60) {
        cachedEvents.clear();
        for (int i = 0; i < 7; ++i) {
            Day day = static_cast<Day>((static_cast<int>(PicoTime::dayOfWeek(now)) + i) % 7);
            auto daily = flattenScheduleForDay(day);

            for (const auto& e : daily) {
                struct tm t = PicoTime::nowTm();
                t.tm_hour = e.start.hour;
                t.tm_min = e.start.minute;
                t.tm_sec = 0;
                t.tm_mday += i;  // Advance day by offset
                time_t when = mktime(&t);
                if (when > static_cast<time_t>(now)) {
                    ProgramEvent future = e;
                    cachedEvents.push_back(future);
                }
            }
        }
        std::sort(cachedEvents.begin(), cachedEvents.end(), [&](const ProgramEvent& a, const ProgramEvent& b) {
            struct tm ta = PicoTime::nowTm();
            struct tm tb = ta;
            ta.tm_hour = a.start.hour; ta.tm_min = a.start.minute; ta.tm_sec = 0;
            tb.tm_hour = b.start.hour; tb.tm_min = b.start.minute; tb.tm_sec = 0;
            return mktime(&ta) < mktime(&tb);
        });
        lastNow = now;
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


