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
#include <unordered_map>

#include "Configuration.hpp"
#include "Display.hpp"
#include "LcdBarGraph.hpp"
#include "Peripherals.hpp"
#include "Sensors.hpp"

namespace Display
{
    namespace Utils
    {
        static std::unordered_map<void( * )(), std::chrono::system_clock::time_point> timers{};
        auto periodic( std::chrono::system_clock::duration interval, void( *func )() ) -> void
        {
            auto now{std::chrono::system_clock::now()};
            auto timer{timers.find( func )};
            if( timer != timers.end() )
            {
                if( now >= timer->second )
                {
                    func();
                    timer->second = now + interval;
                }
            }
            else
            {
                func();
                timers.insert( { func, now + interval } );
            }
        }
    }

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
    static std::future<void> updateFuture{};
    static std::future<void> checkFuture{};
    static std::future<void> ledFuture{};
    static std::mutex mutex{};

    static auto update() -> void
    {
        std::lock_guard<std::mutex> lock{mutex}, i2cLock{Peripherals::i2cMutex};
        //std::lock_guard<std::mutex> lock2{Peripherals::i2cMutex};

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

                //lcd.setCursor(nameMaxLength + 1, nRow);
                bar.draw( n, nameMaxLength + 1, nRow, 20 - ( nameMaxLength + 1 ), map( random( 0, 100 ), cfg.sensors[n].min, cfg.sensors[n].max, 0.0, 100.0 ) );
                nRow++;
            }
        }
    }

    static auto check() -> void
    {
        std::lock_guard<std::mutex> lock{mutex}, i2cLock{Peripherals::i2cMutex};

        static auto ignoreTimer{ std::chrono::system_clock::time_point::min() };
        if ( digitalRead( Peripherals::Pins::BTN ) == LOW )
        {
            for ( uint8_t n = 0; n < states.size(); n++ )
            {
                states[n].warningBuzzer = false;
            }
            //digitalWrite(WRN_BUZZER, LOW);
            ignoreTimer = std::chrono::system_clock::now() + std::chrono::minutes( 10 );
        }

        uint8_t nRow = 0;
        for ( uint8_t n = 0; n < states.size(); n++ )
        {
            if ( cfg.sensors[n].enabled )
            {
                const auto percentage{ map( Sensors::getValue( n ), cfg.sensors[n].min, cfg.sensors[n].max, 0, 100 ) };
                const auto bottom{ cfg.sensors[n].alarm.value };
                const auto top{ ( cfg.sensors[n].alarm.value * 1.05f ) < 100.0f ? ( cfg.sensors[n].alarm.value * 1.05f ) : 100.0f };

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

    static auto led() -> void
    {
        std::lock_guard<std::mutex> lock{mutex};

        auto warningLed{false};
        auto warningBuzzer{false};

        for ( uint8_t n{0}; n < states.size(); n++ )
        {
            if ( cfg.sensors[n].enabled )
            {
                warningLed = warningLed || ( states[n].warningLed );
                warningBuzzer = warningBuzzer || states[n].warningBuzzer;
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

        //if (warningBuzzer)
        //{
        //    digitalWrite(WRN_BUZZER, not digitalRead(WRN_BUZZER));
        //}
        //else
        //{
        //    digitalWrite(WRN_BUZZER, LOW);
        //}
    }

    /*
    static auto updateTask() -> void
    {
        auto timePoint{std::chrono::system_clock::now()};
        while ( 1 )
        {
            update();

            timePoint += std::chrono::milliseconds( 500 );
            std::this_thread::sleep_until( timePoint );
        }
    }

    static auto checkTask() -> void
    {
        auto timePoint{std::chrono::system_clock::now()};
        while ( 1 )
        {
            check();

            timePoint += std::chrono::milliseconds( 250 );
            std::this_thread::sleep_until( timePoint );
        }
    }

    static auto ledTask() -> void
    {
        auto timePoint{std::chrono::system_clock::now()};
        while ( 1 )
        {
            led();

            timePoint += std::chrono::milliseconds( 750 );
            std::this_thread::sleep_until( timePoint );
        }
    }
    */

    auto init() -> void
    {
        {
            std::lock_guard<std::mutex> lock{mutex}, i2cLock{Peripherals::i2cMutex};

            Wire.setClock( 100000 );
            lcd.begin( 20, 4 );
            lcd.home();

            bar.init();
        }
        //updateFuture = std::async( std::launch::async, Display::updateTask );
        //checkFuture = std::async( std::launch::async, Display::checkTask );
        //ledFuture = std::async( std::launch::async, Display::ledTask );
    }

    auto process() -> void
    {
        Utils::periodic( std::chrono::milliseconds( 500 ), Display::update );
    }
} // namespace Display