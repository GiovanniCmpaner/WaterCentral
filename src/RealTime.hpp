#pragma once

#include <Arduino.h>

#include <RtcDateTime.h>
#include <functional>
#include <string>

namespace RealTime
{
    auto init() -> void;
    auto now() -> RtcDateTime;
    auto adjust(const RtcDateTime& dateTime) -> void;
    auto onAdjust(std::function<void()> callback) -> void;

    auto stringToDateTime(const std::string &str) -> RtcDateTime;
    auto dateTimeToString(const RtcDateTime &dateTime) -> std::string;
} // namespace RealTime