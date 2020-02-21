#include <Arduino.h>

#include <BME280I2C.h>
#include <LiquidCrystal_I2C.h>
#include <chrono>
#include <cstdint>
#include <future>

#include "Configuration.hpp"
#include "Display.hpp"
#include "LcdBarGraph.hpp"
#include "Peripherals.hpp"

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

    static LcdBarGraph bar{&lcd, 5, 6, 7};

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

        bar.init();
        bar.draw(0, 1, 0, 1, 30.0);
        bar.draw(1, 1, 1, 1, 57.0);
        bar.draw(2, 1, 2, 1, 60.0);
        bar.draw(3, 1, 3, 1, 0.0);

        //std::async(std::launch::async, Display::process);
    }
} // namespace Display