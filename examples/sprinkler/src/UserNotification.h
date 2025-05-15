#pragma once

enum class UserNotification : uint8_t {
  
    RunZoneStart = 0x01,
    RunZoneStop = 0x02,
    RunZoneStarted = 0x03,
    ZoneStarted = 0x04,
    ZoneStopped = 0x05,
    RunZoneCompleted = 0x06,
    ProgramStarted = 0x07,
    ProgramCompleted = 0x08,
    RunProgram = 0x09
};



