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

namespace RealTime
{
    static RtcDS3231<TwoWire> rtc{Wire};
    static std::mutex mutex{};
    static std::future<void> alarmFuture{};
    static std::future<void> syncFuture{};

    static auto syncDateTime() -> void
    {
        std::lock_guard<std::mutex> lock{mutex}, i2cLock{Peripherals::i2cMutex};

        const auto time{timeval{static_cast<std::time_t>(rtc.GetDateTime().Epoch32Time())}};
        settimeofday(&time, nullptr);
    }

    static auto startHardware() -> void
    {
        std::lock_guard<std::mutex> lock{mutex}, i2cLock{Peripherals::i2cMutex};

        rtc.Begin();

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
    }

    static auto configureAlarms() -> void
    {
        std::lock_guard<std::mutex> lock{mutex}, i2cLock{Peripherals::i2cMutex};

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

    static auto checkAlarms() -> void
    {
        std::lock_guard<std::mutex> lock{mutex}, i2cLock{Peripherals::i2cMutex};

        if (cfg.autoSleepWakeUp.enabled)
        {
            const auto flags{rtc.LatchAlarmsTriggeredFlags()};
            if (flags & DS3231AlarmFlag_Alarm2)
            {
                esp_deep_sleep_start();
            }
        }
    }

    static auto alarmTask() -> void
    {
        auto timePoint{std::chrono::system_clock::now()};
        while (1)
        {
            checkAlarms();

            timePoint += std::chrono::minutes(1);
            std::this_thread::sleep_until(timePoint);
        }
    }

    static auto syncTask() -> void
    {
        auto timePoint{std::chrono::system_clock::now()};
        while (1)
        {
            syncDateTime();

            timePoint += std::chrono::minutes(5);
            std::this_thread::sleep_until(timePoint);
        }
    }

    auto init() -> void
    {
        log_d("begin");

        startHardware();
        configureAlarms();
        syncDateTime();

        log_d("now = %s",RealTime::dateTimeToString(std::chrono::system_clock::now()).data());

        log_d("end");
        alarmFuture = std::async(std::launch::async, alarmTask);
        syncFuture = std::async(std::launch::async, syncTask);
    }

    auto adjustDateTime(const std::chrono::system_clock::time_point& timePoint) -> void
    {
        std::lock_guard<std::mutex> lock{mutex}, i2cLock{Peripherals::i2cMutex};

        const auto time{timeval{std::chrono::system_clock::to_time_t(timePoint)}};
        settimeofday(&time, nullptr);

        auto rtcDateTime{RtcDateTime{}};
        rtcDateTime.InitWithEpoch32Time(time.tv_sec);
        rtc.SetDateTime(rtcDateTime);
    }

    auto stringToDateTime(const std::string& str) -> std::chrono::system_clock::time_point
    {
        auto stream{std::istringstream{str}};
        auto time{std::tm{}};
        stream >> std::get_time(&time, "%Y-%m-%d %H:%M:%S");
        return std::chrono::system_clock::from_time_t(std::mktime(&time));
    }

    auto dateTimeToString(const std::chrono::system_clock::time_point& timePoint) -> std::string
    {
        auto stream{std::ostringstream{}};
        auto time{std::chrono::system_clock::to_time_t(timePoint)};
        stream << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return stream.str();
    }

    auto stringToDateTimeHttp(const std::string& str) -> std::chrono::system_clock::time_point
    {
        auto stream{std::istringstream{str}};
        auto time{std::tm{}};
        stream >> std::get_time(&time, "%a, %d %b %Y %H:%M:%S GMT");
        return std::chrono::system_clock::from_time_t(std::mktime(&time));
    }

    auto dateTimeToStringHttp(const std::chrono::system_clock::time_point& timePoint) -> std::string
    {
        auto stream{std::ostringstream{}};
        auto time{std::chrono::system_clock::to_time_t(timePoint)};
        stream << std::put_time(std::localtime(&time), "%a, %d %b %Y %H:%M:%S GMT");
        return stream.str();
    }

    auto compiledDateTime() -> std::chrono::system_clock::time_point
    {
        auto stream{std::stringstream{}};
        stream << __DATE__ << ' ' << __TIME__;
        auto time{std::tm{}};
        stream >> std::get_time(&time, "%b %d %Y %H:%M:%S");
        return std::chrono::system_clock::from_time_t(std::mktime(&time));
    }

    auto ceil(const std::chrono::system_clock::time_point& timePoint, const std::chrono::seconds& duration) -> std::chrono::system_clock::time_point
    {
        return std::chrono::system_clock::from_time_t(((std::chrono::system_clock::to_time_t(timePoint) + duration.count() - 1 ) / duration.count() ) * duration.count());
    }

    auto floor(const std::chrono::system_clock::time_point& timePoint, const std::chrono::seconds& duration) -> std::chrono::system_clock::time_point
    {
        return std::chrono::system_clock::from_time_t((std::chrono::system_clock::to_time_t(timePoint) / duration.count() ) * duration.count());
    }

    auto round(const std::chrono::system_clock::time_point& timePoint, const std::chrono::seconds& duration) -> std::chrono::system_clock::time_point
    {
        return std::chrono::system_clock::from_time_t(((std::chrono::system_clock::to_time_t(timePoint) + duration.count() / 2 ) / duration.count() ) * duration.count());
    }

} // namespace RealTime