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

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"

namespace RealTime
{
    static auto rtc(RtcDS3231<TwoWire>{Wire});
    static auto taskHandle{TaskHandle_t{}};

    static auto checkDateTime() -> void
    {
        const auto compiled{RtcDateTime(__DATE__, __TIME__)};

        if (not rtc.IsDateTimeValid())
        {
            if (rtc.LastError() != I2C_ERROR_OK)
            {
                log_e("rtc error: %d", rtc.LastError());
                std::abort();
            }
            else
            {
                log_d("rtc lost confidence");
                rtc.SetDateTime(compiled);
            }
        }

        if (not rtc.GetIsRunning())
        {
            log_d("starting rtc");
            rtc.SetIsRunning(true);
        }

        const auto now{rtc.GetDateTime()};
        if (now < compiled)
        {
            log_d("updating rtc");
            rtc.SetDateTime(compiled);
        }

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
                    esp_deep_sleep_start();
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
        xTaskCreate(process, "RealTime::process", 2048, nullptr, 0, &taskHandle);
    }

    auto now() -> RtcDateTime
    {
        return rtc.GetDateTime();
    }
} // namespace RealTime