#include <Arduino.h>

#include <esp_log.h>
#include <esp32-hal.h>
#include <esp_sleep.h>
#include <esp_pthread.h>
#include <SPIFFS.h>
#include <HTTPClient.h>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include "WebInterface.hpp"
#include "Display.hpp"
#include "Infos.hpp"
#include "Button.hpp"

Button button{Peripherals::BTN};

void setup()
{
    delay( 1000 );
    Serial.begin( 115200 );
    Serial.setDebugOutput( true );
    log_d( "begin" );

    Peripherals::init();
    Configuration::init();
    Display::init();

    Configuration::load( &cfg );

    RealTime::init();
    Database::init();
    WebInterface::init();
    Infos::init();

    button.onPress( Display::ignore );

    log_d( "end" );
}

void loop()
{
    Infos::process();
    Database::process();
    RealTime::process();
    WebInterface::process();
    Display::process();
    button.process();
}