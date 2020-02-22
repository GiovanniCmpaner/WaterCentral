#include <Arduino.h>

#include <ResponsiveAnalogRead.h>
#include <array>
#include <chrono>
#include <future>
#include <thread>
#include <Wire.h>
#include <BME280I2C.h>

#include "Configuration.hpp"
#include "Display.hpp"
#include "Peripherals.hpp"
#include "Sensors.hpp"

namespace Sensors
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
        std::mutex mutex;
        std::future<void> future;
    };


    static BME280I2C bme{};
    static std::array<Info, 3> infos
    {
        {
            {Peripherals::MPX_DP::VOUT_1, 1.0, 0.0},
            {Peripherals::MPX_DP::VOUT_2, 1.0, 0.0},
            {Peripherals::MPX_DP::VOUT_3, 1.0, 0.0}
        }};
    static constexpr std::array<double, 4> factors{0.0155555555555556, 0.0311111111111111, 0.1555555555555556, 0.2187500000000000};
    static constexpr std::array<double, 4> offsets{2.2222222222222223, 4.4444444444444446, 22.2222222222222250, 31.2500000000000036};

    static auto infoTask(uint8_t index) -> void
    {
        auto timePoint{std::chrono::system_clock::now()};
        while (1)
        {
            {
                std::lock_guard<std::mutex> lock{infos[index].mutex};
                infos[index].analogRead.update();
                const auto rawValue{ infos[index].analogRead.getValue()* infos[index].factor - infos[index].offset };
                const auto calibratedValue{ rawValue* cfg.sensors[index].calibration.factor - cfg.sensors[index].calibration.offset };
                infos[index].value = calibratedValue;
            }
            timePoint += std::chrono::seconds(1);
            std::this_thread::sleep_until(timePoint);
        }
    }

    auto getValue(uint8_t index) -> double
    {
        std::lock_guard<std::mutex> lock{infos[index].mutex};
        return infos[index].value;
    }

    auto init() -> void
    {
        //if(not bme.begin())
        //{
        //    log_e("bme error");
        //    std::abort();
        //}

        for (auto n{0}; n < infos.size(); ++n)
        {
            {
                std::lock_guard<std::mutex> lock{infos[n].mutex};
                infos[n].analogRead.begin(infos[n].pin, false);
                infos[n].factor = factors[cfg.sensors[n].type];
                infos[n].offset = offsets[cfg.sensors[n].type];
            }
            infos[n].future = std::async(std::launch::async, infoTask, n);
        }

        BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
        BME280::PresUnit presUnit(BME280::PresUnit_Pa);

        float temp(NAN), hum(NAN), pres(NAN);
        bme.read(pres, temp, hum, tempUnit, presUnit);
    }
} // namespace Sensors