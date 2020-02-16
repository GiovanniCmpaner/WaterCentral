
#include <esp_log.h>

#include "Peripherals.hpp"

namespace Peripherals
{
    auto init() -> void
    {
        log_d("begin");

        pinMode(Peripherals::SD_CARD::SS, OUTPUT);
        pinMode(Peripherals::SD_CARD::MOSI, OUTPUT);
        pinMode(Peripherals::SD_CARD::MISO, INPUT);
        pinMode(Peripherals::SD_CARD::SCK, OUTPUT);
        pinMode(Peripherals::BME280::SDA, INPUT);
        pinMode(Peripherals::BME280::SCL, INPUT);
        pinMode(Peripherals::PAM8403::RIN, OUTPUT);
        pinMode(Peripherals::DS3231::SQW_INT, INPUT);
        pinMode(Peripherals::DS3231::SDA, INPUT);
        pinMode(Peripherals::DS3231::SCL, INPUT);
        pinMode(Peripherals::MPX_DP::VOUT_1, INPUT);
        pinMode(Peripherals::MPX_DP::VOUT_2, INPUT);
        pinMode(Peripherals::MPX_DP::VOUT_3, INPUT);
        pinMode(Peripherals::BTN, INPUT);
        pinMode(Peripherals::LED_HTB, OUTPUT);
        pinMode(Peripherals::PRF_CTL, OUTPUT);

        digitalWrite(Peripherals::SD_CARD::SS, HIGH);
        digitalWrite(Peripherals::SD_CARD::MOSI, LOW);
        digitalWrite(Peripherals::SD_CARD::SCK, LOW);
        digitalWrite(Peripherals::PAM8403::RIN, LOW);
        digitalWrite(Peripherals::LED_HTB, LOW);
        digitalWrite(Peripherals::PRF_CTL, HIGH);

        log_d("end");
    }
}; // namespace Peripherals