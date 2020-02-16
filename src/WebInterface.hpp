#pragma once

#include <Arduino.h>

namespace WebInterface
{
    auto init() -> bool;
    auto process() -> void;
} // namespace WebInterface