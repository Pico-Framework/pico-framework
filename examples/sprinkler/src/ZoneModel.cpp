#include "ZoneModel.h"
#include "DebugTrace.h"
#include "hardware/gpio.h"
#include "framework/AppContext.h"
#include "events/EventManager.h"
#include "events/TimerService.h"
#include "framework/FrameworkModel.h"
#include "UserNotification.h"

ZoneModel::ZoneModel() : FrameworkModel("/zones.json") {}

bool ZoneModel::startZone(const std::string& name) {
    auto it = nameIndex.find(name);
    if (it == nameIndex.end()) {
        printf("Zone '%s' not found", name.c_str());
        return false;
    }

    Zone* z = it->second;
    gpio_init(z->gpioPin);
    gpio_set_dir(z->gpioPin, GPIO_OUT);
    gpio_put(z->gpioPin, 1);
    z->active = true;

    printf("Zone '%s' started on GPIO %d", z->name.c_str(), z->gpioPin);
    Event e(eventMask(UserNotification::ZoneStarted));
    AppContext::get<EventManager>()->postEvent(e);
    return true;
}

bool ZoneModel::startZone(const std::string& name, uint32_t durationSeconds) {
    if (!startZone(name)) return false;

    time_t when = time(NULL) + durationSeconds;
    AppContext::get<TimerService>()->scheduleCallbackAt(when, [name, durationSeconds]() {
        AppContext::get<ZoneModel>()->stopZone(name);
        printf("Zone '%s' auto-stopped after %u seconds", name.c_str(), durationSeconds);
        Event e(eventMask(UserNotification::RunZoneCompleted));
        AppContext::get<EventManager>()->postEvent(e);
    });

    return true;
}



bool ZoneModel::stopZone(const std::string& name) {
    auto it = nameIndex.find(name);
    if (it == nameIndex.end()) {
        printf("Zone '%s' not found", name.c_str());
        return false;
    }

    Zone* z = it->second;
    gpio_put(z->gpioPin, 0);
    z->active = false;

    printf("Zone '%s' stopped on GPIO %d", z->name.c_str(), z->gpioPin);
    Event e(eventMask(UserNotification::ZoneStopped));
    AppContext::get<EventManager>()->postEvent(e);
    return true;
}


void ZoneModel::rebuildNameIndex() {
    nameIndex.clear();
    for (auto& z : zones) {
        nameIndex[z.name] = &z;
    }
}

bool ZoneModel::updateZone(const std::string& name, const Zone& data) {
    auto it = nameIndex.find(name);
    if (it != nameIndex.end()) {
        Zone* z = it->second;
        // id and gpioPin are immutable
        z->name = data.name; // update mutable fields
        z->active = data.active;

        if (z->name != name) {
            rebuildNameIndex(); // name change needs reindex
        }

        return true;
    }
    return false;
}

bool ZoneModel::isZoneRunning(const std::string& name) const {
    auto it = nameIndex.find(name);
    return it != nameIndex.end() ? it->second->active : false;
}

bool ZoneModel::load() {
    if (!FrameworkModel::load())
        return false;

    zones.clear();
    for (const auto& entry : all()) {
        if (entry.contains("name") && entry.contains("gpioPin")) {
            zones.push_back(entry.get<Zone>());
        } else {
            printf("Invalid zone entry in collection\n");
        }
    }

    rebuildNameIndex();
    printf("Loaded %zu zones\n", zones.size());
    return true;
}


bool ZoneModel::save() {
    collection = zones;  // Implicit json conversion
    return FrameworkModel::save();
}

bool ZoneModel::save(const std::string &id, const json &data) {
    return FrameworkModel::save(id, data);
}



