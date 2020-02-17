#include <Arduino.h>

#include <esp_log.h>
#include <esp_sleep.h>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include "WebInterface.hpp"

void setup()
{
    delay(1000);
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    log_d("begin");

    Peripherals::init();
    Configuration::init();

    cfg.load();

    RealTime::init();
    Database::init();
    WebInterface::init();

    log_d("end");
}

void loop()
{
}