#include <Arduino.h>

#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <esp_log.h>

#include "Peripherals.hpp"

static auto hspi{SPIClass{HSPI}};

void setup()
{
    delay(1000);
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    log_d("begin");

    Peripherals::init();

    if (not SD.begin(Peripherals::SD_CARD::SS, hspi) || SD.cardType() == CARD_NONE)
    {
        log_e("sd fail");
    }

    log_d("end");
}

void loop()
{
}