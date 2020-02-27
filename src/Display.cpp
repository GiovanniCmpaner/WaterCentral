#include <Arduino.h>

#include <BME280I2C.h>
#include <LiquidCrystal_I2C.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <future>
#include <esp_pthread.h>
#include <Wire.h>
#include <functional>

#include "Configuration.hpp"
#include "Display.hpp"
#include "LcdBarGraph.hpp"
#include "Peripherals.hpp"
#include "Infos.hpp"
#include "Utils.hpp"

namespace Display
{
    struct State
    {
        bool warningMode;
        bool warningLed;
        bool warningBuzzer;
        bool blinkHidden;
    };

    static LiquidCrystal_I2C lcd
    {
        Peripherals::LCD::I2C_ADDRESS,
        Peripherals::LCD::Expander::EN,
        Peripherals::LCD::Expander::RW,
        Peripherals::LCD::Expander::RS,
        Peripherals::LCD::Expander::D4,
        Peripherals::LCD::Expander::D5,
        Peripherals::LCD::Expander::D6,
        Peripherals::LCD::Expander::D7,
        Peripherals::LCD::Expander::BACKLIGHT,
        POSITIVE
    };

    static LcdBarGraph bar{&lcd, 5, 6, 7};
    static std::array<State, 3> states{};
    static std::chrono::system_clock::time_point ignoreTimer{};

    static auto update() -> void
    {
        size_t nameMaxLength{0};
        for ( size_t n{0}; n < states.size(); ++n )
        {
            if ( cfg.sensors[n].enabled )
            {
                nameMaxLength = std::max( nameMaxLength, cfg.sensors[n].name.length() );
            }
        }

        uint8_t nRow{0};
        for ( size_t n{0}; n < states.size(); ++n )
        {
            if ( cfg.sensors[n].enabled )
            {
                lcd.setCursor( 0, nRow );

                if ( not states[n].blinkHidden )
                {
                    if ( states[n].warningMode )
                    {
                        states[n].blinkHidden = true;
                    }
                }
                else
                {
                    states[n].blinkHidden = false;
                }

                if ( states[n].blinkHidden )
                {
                    lcd.print( std::string( cfg.sensors[n].name.size(), ' ' ).data() );
                }
                else
                {
                    lcd.print( cfg.sensors[n].name.data() );
                }

                const auto percentage{ map( Infos::getSensor( n ), cfg.sensors[n].min, cfg.sensors[n].max, 0.0, 100.0 ) };
                bar.draw( n, nameMaxLength + 1, nRow, 20 - ( nameMaxLength + 1 ), percentage );
                nRow++;
            }
        }
    }

    static auto check() -> void
    {
        uint8_t nRow = 0;
        for ( uint8_t n = 0; n < states.size(); n++ )
        {
            if ( cfg.sensors[n].enabled and cfg.sensors[n].alarm.enabled )
            {
                const auto percentage{ map( Infos::getSensor( n ), cfg.sensors[n].min, cfg.sensors[n].max, 0.0, 100.0 ) };
                const auto bottom{ cfg.sensors[n].alarm.value };
                const auto top{ min( cfg.sensors[n].alarm.value * 1.05, 100.0 ) };

                if ( percentage < bottom )
                {
                    if ( not states[n].warningMode )
                    {
                        states[n].warningLed = true;
                        if( std::chrono::system_clock::now() >= ignoreTimer )
                        {
                            states[n].warningBuzzer = true;
                        }
                    }
                    states[n].warningMode = true;
                }
                else if( percentage >= top )   //histeresis
                {
                    states[n].warningMode = false;
                    states[n].warningLed = false;
                    states[n].warningBuzzer = false;
                }
                nRow++;
            }
        }
    }

    static auto button() -> void
    {
        if ( digitalRead( Peripherals::Pins::BTN ) == LOW )
        {
            digitalWrite( Peripherals::Pins::WRN_BZR, LOW );

            for ( uint8_t n = 0; n < states.size(); n++ )
            {
                states[n].warningBuzzer = false;
            }
            ignoreTimer = std::chrono::system_clock::now() + std::chrono::minutes( 10 );
        }
    }

    static auto warning() -> void
    {
        auto warningLed{false};
        auto warningBuzzer{false};

        for ( uint8_t n{0}; n < states.size(); n++ )
        {
            if ( cfg.sensors[n].enabled and cfg.sensors[n].alarm.enabled )
            {
                if( states[n].warningLed )
                {
                    warningLed = true;
                }
                if( states[n].warningBuzzer )
                {
                    warningBuzzer = true;
                }
            }
        }

        if ( warningLed )
        {
            digitalWrite( Peripherals::Pins::LED_HTB, not digitalRead( Peripherals::Pins::LED_HTB ) );
        }
        else
        {
            digitalWrite( Peripherals::Pins::LED_HTB, LOW );
        }

        if ( warningBuzzer )
        {
            digitalWrite( Peripherals::Pins::WRN_BZR, not digitalRead( Peripherals::Pins::WRN_BZR ) );
        }
        else
        {
            digitalWrite( Peripherals::Pins::WRN_BZR, LOW );
        }
    }

    auto init() -> void
    {
        lcd.begin( 20, 4 );
        lcd.home();

        bar.init();
    }

    auto process() -> void
    {
        Utils::periodic( std::chrono::milliseconds( 250 ), Display::check );
        Utils::periodic( std::chrono::milliseconds( 500 ), Display::update );
        Utils::periodic( std::chrono::milliseconds( 750 ), Display::warning );
        Utils::periodic( std::chrono::milliseconds( 50 ), Display::button );
    }
} // namespace Display