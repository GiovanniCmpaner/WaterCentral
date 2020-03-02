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
    Configuration::load( &cfg );

    RealTime::init();
    Database::init();
    WebInterface::init();
    Display::init();
    Infos::init();

    button.onPress( []
    {
        Display::ignore();
    } );
    button.onPress( 5000, nullptr, RealTime::sleep );

    log_d( "end" );
}

void loop()
{
    if( RealTime::isRunning() )
    {
        Database::process();
    }
    RealTime::process();
    WebInterface::process();
    Display::process();
    Infos::process();
    button.process();
}