#include <Arduino.h>

#include <cstdint>
#include <chrono>
#include <future>
#include <LiquidCrystal_I2C.h>

#include "Configuration.hpp"
#include "Display.hpp"
#include "Peripherals.hpp"

namespace Display
{
    static auto lcd{LiquidCrystal_I2C{0x38}};

    static auto process() -> void 
    {
        while(1)
        {
            
        }
    }

    auto init() -> void
    {
        lcd.begin(20,4); 

        std::async(std::launch::async, Display::process);
    }
} // namespace Display