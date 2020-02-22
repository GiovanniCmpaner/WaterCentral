#include <Arduino.h>

#include <BME280I2C.h>
#include <LiquidCrystal_I2C.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <future>
#include <esp_pthread.h>

#include "Configuration.hpp"
#include "Display.hpp"
#include "LcdBarGraph.hpp"
#include "Peripherals.hpp"
#include "Sensors.hpp"

namespace Display
{
    struct State
    {
        bool warningMode;
        bool warningLed;
        bool warningBuzzer;
        bool blinkHidden;
    };

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
    static std::array<State, 3> states{};
    static std::future<void> future{};

    static auto update() -> void
    {
        size_t nameMaxLength{0};
        for (size_t n{0}; n < states.size(); ++n)
        {
            if (cfg.sensors[n].enabled)
            {
                nameMaxLength = std::max(nameMaxLength, cfg.sensors[n].name.length());
            }
        }

        uint8_t nRow{0};
        for (size_t n{0}; n < states.size(); ++n)
        {
            if (cfg.sensors[n].enabled)
            {
                lcd.setCursor(0, nRow);
                if (states[n].blinkHidden)
                {
                    lcd.print(std::string(cfg.sensors[n].name.size(), ' ').data());
                }
                else
                {
                    lcd.print(cfg.sensors[n].name.data());
                }

                
                //lcd.setCursor(nameMaxLength + 1, nRow);
                //bar.draw(n, nameMaxLength + 1, nRow, 20 - (nameMaxLength + 1), map(random(0,100), cfg.sensors[n].min, cfg.sensors[n].max, 0.0, 100.0));
                nRow++;
            }
        }
    }

    static auto check() -> void
    {
    }

    static auto displayTask() -> void
    {
        auto timePoint{std::chrono::system_clock::now()};
        while (1)
        {
            update();
            check();

            timePoint += std::chrono::seconds(5);
            std::this_thread::sleep_until(timePoint);
        }
    }

    auto init() -> void
    {
        lcd.begin(20, 4);
        lcd.home();

        bar.init();

        future = std::async(std::launch::async, displayTask);
    }
} // namespace Display