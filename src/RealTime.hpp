#pragma once

#include <Arduino.h>

#include <RtcDateTime.h>
#include <functional>
#include <string>
#include <chrono>

namespace RealTime
{
    auto init() -> void;
    auto adjust(const std::chrono::system_clock::time_point& timePoint) -> void;
    auto onAdjust(std::function<void()> callback) -> void;

    auto stringToDateTime(const std::string &str) -> std::chrono::system_clock::time_point;
    auto dateTimeToString(const std::chrono::system_clock::time_point &timePoint) -> std::string;
} // namespace RealTime