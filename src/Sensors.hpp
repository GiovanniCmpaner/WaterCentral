#pragma once

#include <Arduino.h>

namespace Sensors
{
    auto init() -> void;
    auto getValue(uint8_t index) -> double;
}