#pragma once

#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <functional>
#include <chrono>

namespace Database
{
    struct SensorData
    {
        std::int64_t id;
        std::time_t dateTime;
        double temperature;
        double humidity;
        double pressure;
        std::array<double, 3> sensors;

        static auto get() -> SensorData;
        static auto serialize ( ArduinoJson::JsonVariant& json, const SensorData& sensorData ) -> void;
    };

    auto init() -> void;
    auto getSensorsData( 
        std::function<void ( const SensorData& ) > callback, 
        int64_t id = {}, 
        std::chrono::system_clock::time_point start = std::chrono::system_clock::time_point::min(), 
        std::chrono::system_clock::time_point end = std::chrono::system_clock::time_point::max() 
    ) -> void;
} // namespace Database