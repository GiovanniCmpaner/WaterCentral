#pragma once

#include <RtcDateTime.h>
#include <functional>

namespace RealTime
{
    auto init() -> void;
    auto now() -> RtcDateTime;
} // namespace RealTime