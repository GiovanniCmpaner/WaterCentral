#pragma once

#include <Arduino.h>

#include <RtcDateTime.h>
#include <functional>
#include <string>
#include <chrono>

namespace RealTime
{
    auto init() -> void;
    auto process() -> void;
    auto adjustDateTime( const std::chrono::system_clock::time_point& timePoint ) -> void;
    auto sleep() -> void;
    auto isRunning() -> bool;
} // namespace RealTime