
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <cstdlib>
#include <esp_log.h>

#include "Peripherals.hpp"

namespace Peripherals
{
    static auto hspi{SPIClass{HSPI}};

    auto init() -> void
    {
        log_d("begin");

        pinMode(Peripherals::SD_CARD::SS, OUTPUT);
        pinMode(Peripherals::SD_CARD::MOSI, OUTPUT);
        pinMode(Peripherals::SD_CARD::MISO, INPUT_PULLUP);
        pinMode(Peripherals::SD_CARD::SCK, OUTPUT);
        pinMode(Peripherals::BME280::SDA, INPUT_PULLUP);
        pinMode(Peripherals::BME280::SCL, INPUT_PULLUP);
        pinMode(Peripherals::PAM8403::RIN, OUTPUT);
        pinMode(Peripherals::DS3231::SQW_INT, INPUT_PULLUP);
        pinMode(Peripherals::DS3231::SDA, INPUT_PULLUP);
        pinMode(Peripherals::DS3231::SCL, INPUT_PULLUP);
        pinMode(Peripherals::MPX_DP::VOUT_1, INPUT);
        pinMode(Peripherals::MPX_DP::VOUT_2, INPUT);
        pinMode(Peripherals::MPX_DP::VOUT_3, INPUT);
        pinMode(Peripherals::BTN, INPUT_PULLUP);
        pinMode(Peripherals::LED_HTB, OUTPUT);
        pinMode(Peripherals::PRF_CTL, OUTPUT);

        digitalWrite(Peripherals::SD_CARD::SS, HIGH);
        digitalWrite(Peripherals::SD_CARD::MOSI, LOW);
        digitalWrite(Peripherals::SD_CARD::SCK, LOW);
        digitalWrite(Peripherals::PAM8403::RIN, LOW);
        digitalWrite(Peripherals::LED_HTB, LOW);
        digitalWrite(Peripherals::PRF_CTL, HIGH);

        if (not SD.begin(Peripherals::SD_CARD::SS, Peripherals::SD_CARD::hspi) || SD.cardType() == CARD_NONE)
        {
            log_e("sd error");
            std::abort();
        }

        log_d("end");
    }
}; // namespace Peripherals