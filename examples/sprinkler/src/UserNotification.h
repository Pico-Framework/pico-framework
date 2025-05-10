#pragma once

enum class UserNotification : uint8_t {
    SchedulerCheck = 0x01,
    RunZoneStart = 0x02,
    RunZoneStop = 0x03,
    RunZoneStarted = 0x04,
    ZoneStarted = 0x05,
    ZoneStopped = 0x06,
    RunZoneCompleted = 0x07,
    ProgramStarted = 0x08,
    ProgramCompleted = 0x09,
};
