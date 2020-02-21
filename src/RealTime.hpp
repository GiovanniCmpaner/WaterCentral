#pragma once

#include <Arduino.h>

#include <RtcDateTime.h>
#include <functional>
#include <string>
#include <chrono>

namespace RealTime
{
    auto init() -> void;
    auto adjustDateTime(const std::chrono::system_clock::time_point& timePoint) -> void;

    auto stringToDateTime(const std::string &str) -> std::chrono::system_clock::time_point;
    auto dateTimeToString(const std::chrono::system_clock::time_point &timePoint) -> std::string;
    auto stringToDateTimeHttp(const std::string &str) -> std::chrono::system_clock::time_point;
    auto dateTimeToStringHttp(const std::chrono::system_clock::time_point &timePoint) -> std::string;
    auto compiledDateTime() -> std::chrono::system_clock::time_point;
} // namespace RealTime