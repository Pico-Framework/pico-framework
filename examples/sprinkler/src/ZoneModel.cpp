#include "ZoneModel.h"
#include "DebugTrace.h"
#include "hardware/gpio.h"
#include "framework/AppContext.h"
#include "events/EventManager.h"
#include "events/Notification.h"
#include "events/Event.h"
#include "events/TimerService.h"
#include "framework/FrameworkModel.h"
#include "UserNotification.h"
#include "time/PicoTime.h"

ZoneModel::ZoneModel(const std::string &path)
    : FrameworkModel(path) {}

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

    AppContext::get<EventManager>()->postEvent(userEvent(UserNotification::ZoneStarted, name));
    return true;
}


bool ZoneModel::startZone(const std::string& name, uint32_t durationSeconds) {
    if (!startZone(name)) return false;
    AppContext::get<EventManager>()->postEvent(userEvent(UserNotification::RunZoneStarted, name));
    time_t when = PicoTime::now() + durationSeconds;
    time_t now = PicoTime::now();

    struct timespec t;
    aon_timer_get_time(&t);

    std::function<void()> stopCallback = [this, name]() {
        this->stopZone(name);
        AppContext::get<EventManager>()->postEvent(userEvent(UserNotification::RunZoneCompleted, name));
    };
    AppContext::get<TimerService>()->scheduleCallbackAt(when, stopCallback);
    return true;
}

bool ZoneModel::stopZone(const std::string& name) {
    auto it = nameIndex.find(name);
    if (it == nameIndex.end()) {
        printf("Zone '%s' not found\n", name.c_str());
        return false;
    }

    Zone* z = it->second;
    gpio_put(z->gpioPin, 0);
    z->active = false;

    AppContext::get<EventManager>()->postEvent(userEvent(UserNotification::ZoneStopped, name));
    return true;
}


void ZoneModel::rebuildNameIndex() {
    nameIndex.clear();
    for (auto& z : zones) {
        nameIndex[z.name] = &z;
    }
}

bool ZoneModel::updateZone(const std::string& id, const Zone& data) {
    for (auto& z : zones) {
        if (z.id == id) {
            // id and gpioPin are immutable
            z.name = data.name;
            z.active = data.active;
            z.image = data.image; // optional, if updating image too

            rebuildNameIndex(); // in case name changed
            return true;
        }
    }
    return false;
}

bool ZoneModel::updateZoneByName(const std::string& name, const Zone& data) {
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

    for (const auto& entry : FrameworkModel::all()) {
        if (!entry.is_object()) {
            printf("ZoneModel: Skipping non-object entry\n");
            continue;
        }

        Zone z;
        z.id = entry.value("id", "0");
        z.name = entry.value("name", "Unnamed");
        z.name = z.name.substr(0, 32); // limit name length
        z.image = entry.value("image", "default.jpg");
    
        int pin = entry.value("gpioPin", -1);
        if (pin < 0 || pin > 255) {
            printf("ZoneModel: Skipping zone with invalid gpioPin: %d\n", pin);
            continue;
        }
        z.gpioPin = static_cast<uint8_t>(pin);

        z.active = entry.value("active", false);

        printf("ZoneModel: Loaded zone id: %s, %s (GPIO %d), image: %s\n", z.id.c_str(), z.name.c_str(), z.gpioPin, z.image.c_str());
        zones.push_back(z);
        collection.push_back(z);  // keep collection in sync
    }

    rebuildNameIndex();
    printf("[ZoneModel] Loaded %zu zones\n", zones.size());
    return true;
}

bool ZoneModel::save() {
    collection.clear();
    for (const auto& zone : zones) {
        collection.push_back(zone);  // uses to_json from NLOHMANN_DEFINE_TYPE_INTRUSIVE
    }
    return FrameworkModel::save();
}

bool ZoneModel::save(const std::string &id, const json &data) {
    return FrameworkModel::save(id, data);
}



