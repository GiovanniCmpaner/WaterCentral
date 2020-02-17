#pragma once

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <RtcDateTime.h>

struct SensorData
{
    int64_t id;
    RtcDateTime dateTime;
    double temperature;
    double humidity;
    double pressure;
    double sensors[4];

    static auto get() -> SensorData;
    static auto serialize(ArduinoJson::JsonVariant &json, const SensorData &sensorData) -> void;
};