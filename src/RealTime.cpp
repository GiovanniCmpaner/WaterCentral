#include <Arduino.h>

#include <FreeRTOS.h>
#include <RtcDS3231.h>
#include <Wire.h>
#include <cstdlib>
#include <driver/rtc_io.h>
#include <esp_log.h>
#include <esp_sleep.h>
#include <freertos/task.h>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"

namespace RealTime
{
    static auto rtc(RtcDS3231<TwoWire>{Wire});
    static auto taskHandle{TaskHandle_t{}};
    static auto mutexHandle{SemaphoreHandle_t{}};

    static auto checkDateTime() -> void
    {
        const auto compiled{RtcDateTime(__DATE__, __TIME__)};

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

        const auto now{rtc.GetDateTime()};
        log_d("now = %04u-%02u-%02u %02u:%02u:%02u", now.Year(), now.Month(), now.Day(), now.Hour(), now.Minute(), now.Second());
    }

    static auto configureAlarms() -> void
    {
        //const auto now{rtc.GetDateTime()};
        //const auto f1{RtcDateTime{now + 60}};
        //const auto f2{RtcDateTime{now + 120}};
        //cfg.autoSleepWakeUp.sleep.hour = f1.Hour();
        //cfg.autoSleepWakeUp.sleep.minute = f1.Minute();
        //cfg.autoSleepWakeUp.wakeUp.hour = f2.Hour();
        //cfg.autoSleepWakeUp.wakeUp.minute = f2.Minute();

        log_d("enabled = %u", cfg.autoSleepWakeUp.enabled);
        log_d("wakeUp = %02u:%02u:%02u", cfg.autoSleepWakeUp.wakeUp.hour, cfg.autoSleepWakeUp.wakeUp.minute, cfg.autoSleepWakeUp.wakeUp.second);
        log_d("sleep = %02u:%02u:%02u", cfg.autoSleepWakeUp.sleep.hour, cfg.autoSleepWakeUp.sleep.minute, cfg.autoSleepWakeUp.sleep.second);

        if (cfg.autoSleepWakeUp.enabled)
        {
            rtc.Enable32kHzPin(false);
            rtc.SetSquareWavePin(DS3231SquareWavePin_ModeAlarmBoth);
            {
                const auto wakeUpAlarm{DS3231AlarmOne{0,
                                                      cfg.autoSleepWakeUp.wakeUp.hour,
                                                      cfg.autoSleepWakeUp.wakeUp.minute,
                                                      0,
                                                      DS3231AlarmOneControl_HoursMinutesSecondsMatch}};
                rtc.SetAlarmOne(wakeUpAlarm);
            }
            {
                const auto sleepAlarm{DS3231AlarmTwo{0,
                                                     cfg.autoSleepWakeUp.sleep.hour,
                                                     cfg.autoSleepWakeUp.sleep.minute,
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

    static auto process(void *) -> void
    {
        while (1)
        {
            if (cfg.autoSleepWakeUp.enabled)
            {
                const auto flags{rtc.LatchAlarmsTriggeredFlags()};
                if (flags & DS3231AlarmFlag_Alarm2)
                {
                    //esp_deep_sleep_start();
                }
            }
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    auto init() -> void
    {
        log_d("begin");

        rtc.Begin();
        checkDateTime();
        configureAlarms();

        log_d("end");
        mutexHandle = xSemaphoreCreateMutex();
        xTaskCreatePinnedToCore(process, "RealTime::process", 2048, nullptr, 0, &taskHandle, 0);
    }

    auto now() -> RtcDateTime
    {
        auto now{RtcDateTime{}};
        xSemaphoreTake(mutexHandle, portMAX_DELAY);
        now = rtc.GetDateTime();
        xSemaphoreGive(mutexHandle);
        return now;
    }

    auto adjust(const RtcDateTime &dateTime) -> void
    {
        rtc.SetDateTime(dateTime);
    }

    auto stringToDateTime(const std::string &str) -> RtcDateTime
    {
        auto stream{std::istringstream{str}};

        auto year{uint32_t{}};
        auto month{uint32_t{}};
        auto day{uint32_t{}};
        auto hour{uint32_t{}};
        auto minute{uint32_t{}};
        auto second{uint32_t{}};

        auto delimiter{char{}};
        stream >> year >> delimiter;
        stream >> month >> delimiter;
        stream >> day;
        stream >> hour >> delimiter;
        stream >> minute >> delimiter;
        stream >> second;

        return RtcDateTime{static_cast<uint16_t>(year),
                           static_cast<uint8_t>(month),
                           static_cast<uint8_t>(day),
                           static_cast<uint8_t>(hour),
                           static_cast<uint8_t>(minute),
                           static_cast<uint8_t>(second)};
    }

    auto dateTimeToString(const RtcDateTime &dateTime) -> std::string
    {
        auto stream{std::ostringstream{}};

        stream << std::setfill('0') << std::setw(4) << static_cast<uint32_t>(dateTime.Year()) << '-';
        stream << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(dateTime.Month()) << '-';
        stream << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(dateTime.Day()) << ' ';
        stream << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(dateTime.Hour()) << ':';
        stream << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(dateTime.Minute()) << ':';
        stream << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(dateTime.Second());

        return stream.str();
    }
} // namespace RealTime