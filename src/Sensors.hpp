#pragma once

#include <Arduino.h>

namespace Sensors
{
    auto init() -> void;
    auto process() -> void;
    auto getValue( uint8_t index ) -> double;
    auto getPressure() -> double;
    auto getTemperature() -> double;
    auto getHumidity() -> double;
}