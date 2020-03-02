#pragma once

#include <Arduino.h>

namespace Display
{
    auto init() -> void;
    auto process() -> void;
    auto ignore() -> void;
}