#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <mutex>

namespace Peripherals
{
    namespace SD_CARD
    {
        enum Pins
        {
            SS = 15,
            MOSI = 13,
            MISO = 12,
            SCK = 14
        };
    }; // namespace SD_CARD

    namespace BME280
    {
        enum Pins
        {
            SDA = 21,
            SCL = 22
        };
        static constexpr uint8_t I2C_ADDRESS{0x76};
    }; // namespace BME280

    namespace LCD
    {
        enum Pins
        {
            SDA = 21,
            SCL = 22,
        };
        static constexpr uint8_t I2C_ADDRESS{0x3F};

        namespace Expander
        {
            enum Pins
            {
                EN = 2,
                RW = 1,
                RS = 0,
                D4 = 4,
                D5 = 5,
                D6 = 6,
                D7 = 7,
                BACKLIGHT = 3,
            };
        }

    }; // namespace LCD

    namespace DS3231
    {
        enum Pins
        {
            SQW_INT = 34,
            SCL = 22,
            SDA = 21
        };
        static constexpr uint8_t I2C_ADDRESS{0x68};
    }; // namespace DS3231

    namespace MPX_DP
    {
        enum Pins
        {
            VOUT_1 = 33,
            VOUT_2 = 32,
            VOUT_3 = 35
        };
    };

    enum Pins
    {
        BTN = 4,
        LED_HTB = 16,
        PRF_CTL = 27,
        WRN_BZR = 25
    };

    auto init() -> void;
}; // namespace Peripherals