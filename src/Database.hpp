#pragma once

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <RtcDateTime.h>
#include <functional>
#include <chrono>

#include "SensorData.hpp"

namespace Database
{
    auto init() -> void;
    auto getSensorsData(
        std::function<void(const SensorData&)> callback, 
        int64_t id = {},
        std::chrono::system_clock::time_point start = std::chrono::system_clock::time_point::min(), 
        std::chrono::system_clock::time_point end = std::chrono::system_clock::time_point::max()
    ) -> void;
} // namespace Database