#pragma once

#include <Arduino.h>
#include <SPI.h>

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

        static auto hspi{SPIClass{HSPI}};
    }; // namespace SD_CARD

    namespace BME280
    {
        enum Pins
        {
            SDA = 21,
            SCL = 22
        };
    };

    namespace PAM8403
    {
        enum Pins
        {
            RIN = 25
        };
    };

    namespace DS3231
    {
        enum Pins
        {
            SQW_INT = 34,
            SCL = 22,
            SDA = 21
        };
    };

    namespace MPX_DP
    {
        enum Pins
        {
            VOUT_1 = 33,
            VOUT_2 = 35,
            VOUT_3 = 32
        };
    };

    enum Pins
    {
        BTN = 4,
        LED_HTB = 16,
        PRF_CTL = 27
    };

    auto init() -> void;
}; // namespace Peripherals