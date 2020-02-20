#include <Arduino.h>

#include <RtcDS3231.h>
#include <Wire.h>
#include <driver/rtc_io.h>
#include <esp_log.h>
#include <esp_sleep.h>

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include <chrono>
#include <sys/time.h>

namespace RealTime
{
    static auto rtc(RtcDS3231<TwoWire>{Wire});
    static auto future{std::future<void>{}};

    static auto checkDateTime() -> void
    {
        if (not rtc.IsDateTimeValid() && rtc.LastError() != I2C_ERROR_OK)
        {
            log_e("rtc error: %d", rtc.LastError());
            std::abort();
        }

        if (not rtc.GetIsRunning())
        {
            log_d("starting rtc");
            rtc.SetIsRunning(true);
        }

        const auto time{timeval{static_cast<std::time_t>(rtc.GetDateTime().Epoch32Time())}};
        settimeofday(&time, nullptr);

        log_d("now = %s", RealTime::dateTimeToString(std::chrono::system_clock::now()).data());
    }

    static auto configureAlarms() -> void
    {
        //const auto now{rtc.GetDateTime()};
        //const auto f1{RtcDateTime{now + 60}};
        //const auto f2{RtcDateTime{now + 120}};
        //cfg.autoSleepWakeUp.sleepTime.hour = f1.Hour();
        //cfg.autoSleepWakeUp.sleepTime.minute = f1.Minute();
        //cfg.autoSleepWakeUp.wakeUpTime.hour = f2.Hour();
        //cfg.autoSleepWakeUp.wakeUpTime.minute = f2.Minute();

        log_d("enabled = %u", cfg.autoSleepWakeUp.enabled);
        log_d("sleepTime = %02u:%02u", cfg.autoSleepWakeUp.sleepTime[0], cfg.autoSleepWakeUp.sleepTime[1]);
        log_d("wakeUpTime = %02u:%02u", cfg.autoSleepWakeUp.wakeUpTime[0], cfg.autoSleepWakeUp.wakeUpTime[1]);

        if (cfg.autoSleepWakeUp.enabled)
        {
            rtc.Enable32kHzPin(false);
            rtc.SetSquareWavePin(DS3231SquareWavePin_ModeAlarmBoth);
            {
                const auto wakeUpAlarm{DS3231AlarmOne{0,
                                                      cfg.autoSleepWakeUp.wakeUpTime[0],
                                                      cfg.autoSleepWakeUp.wakeUpTime[1],
                                                      0,
                                                      DS3231AlarmOneControl_HoursMinutesSecondsMatch}};
                rtc.SetAlarmOne(wakeUpAlarm);
            }
            {
                const auto sleepAlarm{DS3231AlarmTwo{0,
                                                     cfg.autoSleepWakeUp.sleepTime[0],
                                                     cfg.autoSleepWakeUp.sleepTime[1],
                                                     DS3231AlarmTwoControl_HoursMinutesMatch}};
                rtc.SetAlarmTwo(sleepAlarm);
            }
            rtc.LatchAlarmsTriggeredFlags();
        }
        else
        {
            rtc.Enable32kHzPin(false);
            rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
        }

        rtc_gpio_pullup_en(static_cast<gpio_num_t>(Peripherals::DS3231::SQW_INT));
        esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(Peripherals::DS3231::SQW_INT), LOW);
    }

    static auto process() -> void
    {
        auto timePoint{std::chrono::system_clock::now()};
        while (1)
        {
            if (cfg.autoSleepWakeUp.enabled)
            {
                const auto flags{rtc.LatchAlarmsTriggeredFlags()};
                if (flags & DS3231AlarmFlag_Alarm2)
                {
                    esp_deep_sleep_start();
                }
            }
            timePoint += std::chrono::seconds(1);
            std::this_thread::sleep_until(timePoint);
        }
    }

    auto init() -> void
    {
        log_d("begin");

        rtc.Begin();
        checkDateTime();
        configureAlarms();

        log_d("end");
        future = std::async(std::launch::async, process);
    }

    auto adjust(const std::chrono::system_clock::time_point &timePoint) -> void
    {
        const auto time{timeval{std::chrono::system_clock::to_time_t(timePoint)}};
        settimeofday(&time, nullptr);

        auto rtcDateTime{RtcDateTime{}};
        rtcDateTime.InitWithEpoch32Time(time.tv_sec);
        rtc.SetDateTime(rtcDateTime);
    }

    auto stringToDateTime(const std::string &str) -> std::chrono::system_clock::time_point
    {
        auto stream{std::istringstream{str}};
        auto time{std::tm{}};
        stream >> std::get_time(&time, "%Y-%m-%d %H:%M:%S");
        return std::chrono::system_clock::from_time_t(std::mktime(&time));
    }

    auto dateTimeToString(const std::chrono::system_clock::time_point &timePoint) -> std::string
    {
        auto stream{std::ostringstream{}};
        auto time{std::chrono::system_clock::to_time_t(timePoint)};
        stream << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return stream.str();
    }
} // namespace RealTime