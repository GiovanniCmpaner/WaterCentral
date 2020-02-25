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

        static SPIClass hspi{HSPI};
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
                BACKLIGHPIN = 3,
            };
        }
        
    }; // namespace LCD

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
        static constexpr uint8_t I2C_ADDRESS{0x68};
    }; // namespace DS3231

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

    extern std::mutex i2cMutex;

    auto init() -> void;
}; // namespace Peripherals