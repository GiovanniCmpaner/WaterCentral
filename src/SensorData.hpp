#pragma once

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <RtcDateTime.h>
#include <cstdint>
#include <ctime>

struct SensorData
{
    std::int64_t id;
    std::time_t dateTime;
    double temperature;
    double humidity;
    double pressure;
    double sensors[4];

    static auto get() -> SensorData;
    static auto serialize(ArduinoJson::JsonVariant &json, const SensorData &sensorData) -> void;
};