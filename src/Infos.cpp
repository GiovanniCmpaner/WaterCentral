#include <Arduino.h>

#include <ResponsiveAnalogRead.h>
#include <array>
#include <chrono>
#include <future>
#include <thread>
#include <Wire.h>
#include <BME280I2C.h>
#include <esp_pthread.h>

#include "Configuration.hpp"
#include "Display.hpp"
#include "Peripherals.hpp"
#include "Infos.hpp"
#include "Utils.hpp"

namespace Infos
{
    //-----------------------------------------------------------------------------------
    //namespace SampleCalculus
    //{
    //    constexpr auto fullScaleVoltage{3.9};
    //    constexpr auto fullScaleReading{4095};
    //    constexpr auto voltageReadingFactor{fullScaleVoltage / fullScaleReading};
    //-----------------------------------------------------------------------------------
    //    constexpr auto dividerR1{4700};
    //    constexpr auto dividerR2{10000};
    //    constexpr auto divideVoltage(double inputVoltage) -> double
    //    {
    //        return (inputVoltage * dividerR2) / (dividerR1 + dividerR2);
    //    }
    //-----------------------------------------------------------------------------------
    //
    //    constexpr auto zeroOffsetVoltage{0.2};
    //    constexpr auto sensitivityVoltage{0.045};
    //    constexpr auto scaledZeroOffetVoltage{divideVoltage(zeroOffsetVoltage)};
    //    constexpr auto scaledSensitivityVoltage{divideVoltage(sensitivityVoltage)};
    //    constexpr auto scaledSensitivityVoltage{divideVoltage(sensitivityVoltage)};
    //    constexpr auto scaledSensitivityVoltage{divideVoltage(sensitivityVoltage)};
    //-----------------------------------------------------------------------------------
    //    constexpr auto finalFactor{ voltageReadingFactor / scaledSensitivityVoltage };
    //    constexpr auto finalOffset{ scaledZeroOffetVoltage / scaledSensitivityVoltage };
    //-----------------------------------------------------------------------------------
    //    constexpr auto convertReading(uint16_t reading) -> double
    //    {
    //        return reading * finalFactor - finalOffset;
    //    }
    //-----------------------------------------------------------------------------------
    //    static constexpr auto sampleValue{convertReading(2048)};
    //}; // namespace SampleCalculus
    //-----------------------------------------------------------------------------------

    struct Info
    {
        uint8_t pin;
        double factor;
        double offset;
        double value;
        ResponsiveAnalogRead analogRead;
    };

    static BME280I2C bme{};
    static std::array<Info, 3> infos
    {
        {
            {Peripherals::MPX_DP::VOUT_1, 1.0, 0.0, NAN},
            {Peripherals::MPX_DP::VOUT_2, 1.0, 0.0, NAN},
            {Peripherals::MPX_DP::VOUT_3, 1.0, 0.0, NAN}
        }
    };
    static float pressure{NAN};
    static float temperature{NAN};
    static float humidity{NAN};

    static constexpr std::array<double, 4> factors{0.0155555555555556, 0.0311111111111111, 0.1555555555555556, 0.2187500000000000};
    static constexpr std::array<double, 4> offsets{2.2222222222222223, 4.4444444444444446, 22.2222222222222250, 31.2500000000000036};

    static auto update() -> void
    {
        bme.read( pressure, temperature, humidity, BME280::TempUnit_Celsius, BME280::PresUnit_hPa );
        for ( auto n{0}; n < infos.size(); ++n )
        {
            if( cfg.sensors[n].enabled )
            {
                infos[n].analogRead.update();
                const auto rawValue{ infos[n].analogRead.getValue()* infos[n].factor - infos[n].offset };
                const auto calibratedValue{ rawValue* cfg.sensors[n].calibration.factor - cfg.sensors[n].calibration.offset };
                infos[n].value = calibratedValue;
            }
        }
    }

    auto getSensor( uint8_t index ) -> double
    {
        return infos[index].value;
    }

    auto getPressure() -> double
    {
        return pressure;
    }

    auto getTemperature() -> double
    {
        return temperature;
    }

    auto getHumidity() -> double
    {
        return humidity;
    }

    auto init() -> void
    {
        if( not bme.begin() )
        {
            log_e( "bme error" );
            std::abort();
        }

        for ( auto n{0}; n < infos.size(); ++n )
        {
            if( cfg.sensors[n].enabled )
            {
                infos[n].analogRead.begin( infos[n].pin, false );
                infos[n].factor = factors[cfg.sensors[n].type];
                infos[n].offset = offsets[cfg.sensors[n].type];
            }
        }
    }

    auto process() -> void
    {
        Utils::periodic( std::chrono::seconds( 500 ), Infos::update );
    }

    auto serialize( ArduinoJson::JsonVariant& json ) -> void
    {
        json["temperature"] = Infos::temperature;
        json["humidity"] = Infos::humidity;
        json["pressure"] = Infos::pressure;
        {
            auto sensors{ json["sensors"] };
            for ( auto n{0}; n < Infos::infos.size(); ++n )
            {
                if( cfg.sensors[n].enabled )
                {
                    auto sensor{ sensors.addElement() };

                    sensor["name"] = cfg.sensors[n].name;
                    sensor["value"] = Infos::infos[n].value;
                    sensor["percent"] = map( Infos::infos[n].value, cfg.sensors[n].min, cfg.sensors[n].max, 0.0, 100.0 );
                }
            }
        }
    }
} // namespace Sensors