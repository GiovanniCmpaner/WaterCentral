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

    log_d( "end" );
}
//--------------------------------------
//#include <SPIFFS.h>
//#include <HTTPClient.h>
//#include "AudioFileSourcePROGMEM.h"
//#include "AudioGeneratorWAV.h"
//#include "AudioOutputI2S.h"
//
//std::unique_ptr<AudioGeneratorWAV> wav{};
//std::unique_ptr<AudioFileSourcePROGMEM> file{};
//std::unique_ptr<AudioOutputI2S> out{};
//
//extern const uint8_t cursor_move_wav_start[] asm( "_binary_audio_cursor_move_wav_start" );
//extern const uint8_t cursor_move_wav_end[] asm( "_binary_audio_cursor_move_wav_end" );
//
//void audio()
//{
//    log_d( "begin" );
//
//    pinMode( Peripherals::PRF_CTL, OUTPUT );
//    digitalWrite( Peripherals::PRF_CTL, LOW );
//
//    out.reset( new AudioOutputI2S{ 0, AudioOutputI2S::INTERNAL_DAC } );
//}
////--------------------------------------
//void loop()
//{
//    if ( not wav or not wav->loop() )
//    {
//        wav.reset( new AudioGeneratorWAV{} );
//        file.reset( new AudioFileSourcePROGMEM{ cursor_move_wav_start, cursor_move_wav_end - cursor_move_wav_start } );
//        if( not wav->begin( file.get(), out.get() ) )
//        {
//            log_d( "fail" );
//        }
//    }
//}

void loop()
{
    RealTime::process();
    Database::process();
    WebInterface::process();
    Display::process();
    Infos::process();
}