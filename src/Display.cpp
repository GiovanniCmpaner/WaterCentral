#include <Arduino.h>

#include <LiquidCrystal_I2C.h>
#include <chrono>
#include <cstdint>
#include <future>

#include "Configuration.hpp"
#include "Display.hpp"
#include "Peripherals.hpp"
#include <BME280I2C.h>

namespace Display
{
    static LiquidCrystal_I2C lcd{
        Peripherals::LCD::I2C_ADDRESS,
        Peripherals::LCD::Expander::Pins::EN,
        Peripherals::LCD::Expander::Pins::RW,
        Peripherals::LCD::Expander::Pins::RS,
        Peripherals::LCD::Expander::Pins::D4,
        Peripherals::LCD::Expander::Pins::D5,
        Peripherals::LCD::Expander::Pins::D6,
        Peripherals::LCD::Expander::Pins::D7,
        Peripherals::LCD::Expander::Pins::BACKLIGHPIN,
        POSITIVE};

    static auto process() -> void
    {
        while (1)
        {
        }
    }

    auto init() -> void
    {
        lcd.begin(20, 4);
        lcd.home();
        lcd.print("Hello, ARDUINO ");
        lcd.setCursor(0, 1);
        lcd.print(" FORUM - fm   ");

        //std::async(std::launch::async, Display::process);
    }
} // namespace Display