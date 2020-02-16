#include <Arduino.h>

#include <esp_log.h>

#include "Configuration.hpp"
#include "Peripherals.hpp"
#include "WebInterface.hpp"

void setup()
{
    delay(1000);
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    log_d("begin");

    Peripherals::init();
    Configuration::init();
    if (not cfg.load())
    {
        log_e("config error");
        return;
    }

    WebInterface::init();

    log_d("end");
}

void loop()
{
    WebInterface::process();
}