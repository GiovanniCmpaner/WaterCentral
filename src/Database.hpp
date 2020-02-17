#pragma once

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <RtcDateTime.h>
#include <functional>

#include "SensorData.hpp"

namespace Database
{
    auto init() -> void;
    auto getSensorsData(std::function<void(const SensorData&)> callback, int64_t id = {}, RtcDateTime start = {}, RtcDateTime end = {}) -> uint32_t;
} // namespace Database