#pragma once

#include <Arduino.h>

#include <ArduinoJson.hpp>

namespace Infos
{
    auto init() -> void;
    auto process() -> void;
    auto getSensor( uint8_t index ) -> double;
    auto getPressure() -> double;
    auto getTemperature() -> double;
    auto getHumidity() -> double;

    auto serialize( ArduinoJson::JsonVariant& json ) -> void;
}