#include <Arduino.h>

#include <esp_log.h>
#include <esp32-hal.h>
#include <esp_sleep.h>
#include <esp_pthread.h>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include "WebInterface.hpp"
#include "Display.hpp"
#include "Sensors.hpp"

void setup()
{
    delay(1000);
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    log_d("begin");

    {
        esp_pthread_cfg_t cfg;
        esp_pthread_get_cfg(&cfg);
        cfg.stack_size = 4096;
        esp_pthread_set_cfg(&cfg);
    }

    Peripherals::init();
    
    Configuration::init();
    Configuration::load(&cfg);

    RealTime::init();
    Database::init();
    WebInterface::init();

    digitalWrite(Peripherals::PRF_CTL,LOW);
    Display::init();
    //Sensors::init();

    log_d("end");
}

void loop()
{
}