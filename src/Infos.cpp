#include <Arduino.h>

#include <ResponsiveAnalogRead.h>
#include <array>
#include <chrono>
#include <future>
#include <thread>
#include <Wire.h>
#include <BME280I2C.h>
#include <map>
#include <algorithm>

#include "Configuration.hpp"
#include "Display.hpp"
#include "Peripherals.hpp"
#include "Infos.hpp"
#include "Utils.hpp"

namespace Infos
{
    struct Info
    {
        uint8_t pin;
        double value;
        ResponsiveAnalogRead analogRead;
    };

    static BME280I2C bme{};
    static std::array<Info, 3> infos
    {
        {
            {Peripherals::MPX_DP::VOUT_1},
            {Peripherals::MPX_DP::VOUT_2},
            {Peripherals::MPX_DP::VOUT_3}
        }
    };
    static float pressure{NAN};
    static float temperature{NAN};
    static float humidity{NAN};

    static auto update() -> void
    {
        bme.read( pressure, temperature, humidity, BME280::TempUnit_Celsius, BME280::PresUnit_hPa );
        for ( auto n{0}; n < infos.size(); ++n )
        {
            if( cfg.sensors[n].enabled )
            {
                static constexpr std::array<double, 4> sensorsMax{50.0, 100.0, 500.0, 700.0};
                static constexpr std::array<double, 4> sensorsSensivity{0.0900, 0.0450, 0.0090, 0.0064};
                static constexpr std::array<double, 4> sensorsZeroOffset{-0.2000, -0.2000, -0.2000, -0.2000};

                infos[n].analogRead.update();
                const auto rawRead{infos[n].analogRead.getValue()};
                const auto calibratedRead{ rawRead* cfg.sensors[n].calibration.angularCoefficient + cfg.sensors[n].calibration.linearCoefficient };
                const auto sensorValue{ ( calibratedRead + sensorsZeroOffset[cfg.sensors[n].type] ) / sensorsSensivity[cfg.sensors[n].type] };
                infos[n].value = constrain( sensorValue, 0.0, sensorsMax[cfg.sensors[n].type] );
            }
        }
    }

    auto getSensor( uint8_t index ) -> double
    {
        return ( round( infos[index].value * 100 ) / 100 );
    }

    auto getPressure() -> double
    {
        return ( round( pressure * 100 ) / 100 );
    }

    auto getTemperature() -> double
    {
        return ( round( temperature * 100 ) / 100 );
    }

    auto getHumidity() -> double
    {
        return ( round( humidity * 100 ) / 100 );
    }

    auto init() -> void
    {
        if( not bme.begin() )
        {
            log_d( "bme error" );
        }

        for ( auto n{0}; n < infos.size(); ++n )
        {
            if( cfg.sensors[n].enabled )
            {
                infos[n].analogRead.begin( infos[n].pin, false );
                infos[n].analogRead.setAnalogResolution( 4096 );
            }
        }
    }

    auto process() -> void
    {
        Utils::periodic( std::chrono::milliseconds( 500 ), Infos::update );
    }

    auto serialize( ArduinoJson::JsonVariant& json ) -> void
    {
        json["temperature"] = Infos::getTemperature();
        json["humidity"] = Infos::getHumidity();
        json["pressure"] = Infos::getPressure();
        {
            auto sensors{ json["sensors"] };
            for ( auto n{0}; n < Infos::infos.size(); ++n )
            {
                if( cfg.sensors[n].enabled )
                {
                    auto sensor{ sensors.addElement() };

                    sensor["name"] = cfg.sensors[n].name;
                    sensor["value"] = Infos::getSensor( n );
                    sensor["percent"] = map( Infos::getSensor( n ), cfg.sensors[n].min, cfg.sensors[n].max, 0.0, 100.0 );
                }
            }
        }
    }
} // namespace Sensors