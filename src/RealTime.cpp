#include <Arduino.h>

#include <RtcDS3231.h>
#include <Wire.h>
#include <driver/rtc_io.h>
#include <esp_log.h>
#include <esp_sleep.h>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <thread>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include "Utils.hpp"

namespace RealTime
{
    static RtcDS3231<TwoWire> rtc{Wire};

    static auto syncDateTime() -> void
    {
        const auto time{timeval{static_cast<std::time_t>( rtc.GetDateTime().Epoch32Time() )}};
        settimeofday( &time, nullptr );
    }

    static auto startHardware() -> void
    {
        rtc.Begin();

        if ( not rtc.IsDateTimeValid() && rtc.LastError() != I2C_ERROR_OK )
        {
            log_d( "rtc error: %d", rtc.LastError() );
        }

        rtc.SetIsRunning( true );
    }

    static auto configureAlarms() -> void
    {
        log_d( "enabled = %u", cfg.autoSleepWakeUp.enabled );
        log_d( "sleepTime = %02u:%02u", cfg.autoSleepWakeUp.sleepTime[0], cfg.autoSleepWakeUp.sleepTime[1] );
        log_d( "wakeUpTime = %02u:%02u", cfg.autoSleepWakeUp.wakeUpTime[0], cfg.autoSleepWakeUp.wakeUpTime[1] );

        if ( cfg.autoSleepWakeUp.enabled )
        {
            rtc.Enable32kHzPin( false );
            rtc.SetSquareWavePin( DS3231SquareWavePin_ModeAlarmBoth );
            {
                const auto wakeUpAlarm{DS3231AlarmOne{0,
                                                      cfg.autoSleepWakeUp.wakeUpTime[0],
                                                      cfg.autoSleepWakeUp.wakeUpTime[1],
                                                      0,
                                                      DS3231AlarmOneControl_HoursMinutesSecondsMatch}};
                rtc.SetAlarmOne( wakeUpAlarm );
            }
            {
                const auto sleepAlarm{DS3231AlarmTwo{0,
                                                     cfg.autoSleepWakeUp.sleepTime[0],
                                                     cfg.autoSleepWakeUp.sleepTime[1],
                                                     DS3231AlarmTwoControl_HoursMinutesMatch}};
                rtc.SetAlarmTwo( sleepAlarm );
            }
            rtc.LatchAlarmsTriggeredFlags();
        }
        else
        {
            rtc.Enable32kHzPin( false );
            rtc.SetSquareWavePin( DS3231SquareWavePin_ModeNone );
        }

        esp_sleep_enable_ext0_wakeup( static_cast<gpio_num_t>( Peripherals::DS3231::SQW_INT ), 0 );
        //esp_sleep_enable_ext0_wakeup( static_cast<gpio_num_t>( Peripherals::BTN ), 0 );
    }

    static auto checkSleep() -> void
    {
        if ( cfg.autoSleepWakeUp.enabled )
        {
            const auto flags{rtc.LatchAlarmsTriggeredFlags()};
            if ( flags & DS3231AlarmFlag_Alarm2 )
            {
                sleep();
            }
        }
    }

    auto sleep() -> void
    {
        rtc.SetSquareWavePin( DS3231SquareWavePin_ModeAlarmOne );
        esp_deep_sleep_start();
    }

    auto init() -> void
    {
        log_d( "begin" );

        startHardware();
        configureAlarms();
        syncDateTime();

        log_d( "now = %s", Utils::DateTime::toString( std::chrono::system_clock::now() ).data() );

        log_d( "end" );
    }

    auto adjustDateTime( const std::chrono::system_clock::time_point& timePoint ) -> void
    {
        RtcDateTime rtcDateTime{};
        rtcDateTime.InitWithEpoch32Time( std::chrono::system_clock::to_time_t( timePoint ) );
        rtc.SetDateTime( rtcDateTime );
        rtc.SetIsRunning( true );
    }

    auto process() -> void
    {
        Utils::periodic( std::chrono::minutes( 5 ), RealTime::syncDateTime );
        Utils::periodic( std::chrono::minutes( 1 ), RealTime::checkSleep );
    }
} // namespace RealTime