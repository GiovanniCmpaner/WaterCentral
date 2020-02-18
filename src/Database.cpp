#include <Arduino.h>

#include <FS.h>
#include <FreeRTOS.h>
#include <SD.h>
#include <cstdlib>
#include <esp_log.h>
#include <freertos/task.h>
#include <sqlite3.h>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include "SensorData.hpp"

namespace Database
{
    static auto db{(sqlite3 *){}};
    static auto taskHandle{TaskHandle_t{}};
    static auto future{RtcDateTime{}};

    static auto createTable() -> void
    {
        log_d("begin");
        {
            const auto query{"CREATE TABLE IF NOT EXISTS               "
                             "    SENSORS_DATA (                       "
                             "        ID          INTEGER PRIMARY KEY, "
                             "        DATE_TIME   DATETIME,            "
                             "        TEMPERATURE NUMERIC,             "
                             "        HUMIDITY    NUMERIC,             "
                             "        PRESSURE    NUMERIC,             "
                             "        SENSOR_1    NUMERIC,             "
                             "        SENSOR_2    NUMERIC,             "
                             "        SENSOR_3    NUMERIC,             "
                             "        SENSOR_4    NUMERIC              "
                             "    )                                    "};

            const auto rc{sqlite3_exec(db, query, nullptr, nullptr, nullptr)};
            if (rc != SQLITE_OK)
            {
                log_e("table create error: %s\n", sqlite3_errmsg(db));
                std::abort();
            }
        }
        {
            const auto query{"CREATE UNIQUE INDEX IF NOT EXISTS DATE_TIME_INDEX "
                             "ON SENSORS_DATA( DATE_TIME )                      "};

            const auto rc{sqlite3_exec(db, query, nullptr, nullptr, nullptr)};
            if (rc != SQLITE_OK)
            {
                log_e("table index error: %s\n", sqlite3_errmsg(db));
                std::abort();
            }
        }
        log_d("end");
    }

    static auto insertSensorData(const SensorData &sensorData) -> int64_t
    {
        auto id{int64_t{}};
        log_d("begin");

        const auto query{
            "INSERT INTO SENSORS_DATA ( "
            "    DATE_TIME,             "
            "    TEMPERATURE,           "
            "    HUMIDITY,              "
            "    PRESSURE,              "
            "    SENSOR_1,              "
            "    SENSOR_2,              "
            "    SENSOR_3,              "
            "    SENSOR_4               "
            ")                          "
            "VALUES                     "
            "    (?,?,?,?,?,?,?,?)     "};

        auto res{(sqlite3_stmt *){}};
        const auto rc{sqlite3_prepare_v2(db, query, strlen(query), &res, nullptr)};
        if (rc != SQLITE_OK)
        {
            log_d("insert prepare error: %s", sqlite3_errmsg(db));
            //std::abort();
        }
        else {
            sqlite3_bind_int64(res, 1, sensorData.dateTime.Epoch32Time());
            sqlite3_bind_double(res, 2, sensorData.temperature);
            sqlite3_bind_double(res, 3, sensorData.humidity);
            sqlite3_bind_double(res, 4, sensorData.pressure);
            sqlite3_bind_double(res, 5, sensorData.sensors[0]);
            sqlite3_bind_double(res, 6, sensorData.sensors[1]);
            sqlite3_bind_double(res, 7, sensorData.sensors[2]);
            sqlite3_bind_double(res, 8, sensorData.sensors[3]);

            if (sqlite3_step(res) != SQLITE_DONE)
            {
                log_d("insert error: %s", sqlite3_errmsg(db));
                //std::abort();
            }
            sqlite3_finalize(res);
            id = sqlite3_last_insert_rowid(db);
        }
        log_d("end");
        return id;
    }

    static auto scheduleInsert() -> void
    {
        log_d("begin");

        const auto now{RealTime::now()};
        if (now.IsValid())
        {
            future = RtcDateTime{((now + 59) / 60) * 60};
        }
        else
        {
            future = RtcDateTime{};
        }

        log_d("end");
    }

    static auto process(void *) -> void
    {
        vTaskDelay(pdMS_TO_TICKS(10000));

        const auto interval{pdMS_TO_TICKS(60000)};
        auto lastWakeTime{xTaskGetTickCount()};
        while (1)
        {
            if (future.IsValid())
            {
                const auto now{RealTime::now()};
                if (now >= future)
                {
                    insertSensorData(SensorData::get());
                    scheduleInsert();
                }
            }
            else
            {
                scheduleInsert();
            }
            vTaskDelayUntil(&lastWakeTime, interval);
        }
    }

    auto init() -> void
    {
        log_d("begin");

        sqlite3_initialize();

        const auto rc{sqlite3_open("/sd/sensors_data.db", &db)};
        if (rc != SQLITE_OK)
        {
            log_e("database open error: %s\n", sqlite3_errmsg(db));
            std::abort();
        };

        createTable();

        log_d("end");
        xTaskCreatePinnedToCore(process, "Database::process", 4096, nullptr, 0, &taskHandle, 0);
    }

    auto getSensorsData(std::function<void(const SensorData &)> callback, int64_t id, RtcDateTime start, RtcDateTime end) -> uint32_t
    {
        log_d("begin");

        auto recordCount{uint32_t{}};

        const auto query{
            "SELECT                                   "
            "    ID,                                  "
            "    DATE_TIME,                           "
            "    TEMPERATURE,                         "
            "    HUMIDITY,                            "
            "    PRESSURE,                            "
            "    SENSOR_1,                            "
            "    SENSOR_2,                            "
            "    SENSOR_3,                            "
            "    SENSOR_4                             "
            "FROM                                     "
            "    SENSORS_DATA                         "};
            //"WHERE                                    "
            //"    ( ID >= IFNULL(?,ID) )               "
            //"AND ( DATE_TIME >= IFNULL(?,DATE_TIME) ) "
            //"AND ( DATE_TIME <= IFNULL(?,DATE_TIME) ) "};

        auto res{(sqlite3_stmt *){}};
        const auto rc{sqlite3_prepare_v2(db, query, strlen(query), &res, nullptr)};
        if (rc != SQLITE_OK)
        {
            log_e("select prepare error: %s", sqlite3_errmsg(db));
            std::abort();
        }

        if (id != 0)
        {
            sqlite3_bind_int64(res, 1, id);
        }
        if (start.IsValid())
        {
            sqlite3_bind_int64(res, 2, start.Epoch32Time());
        }
        if (end.IsValid())
        {
            sqlite3_bind_int64(res, 3, end.Epoch32Time());
        }

        while (sqlite3_step(res) == SQLITE_ROW)
        {
            auto sensorData{SensorData{}};
            sensorData.id = sqlite3_column_int64(res, 0);
            sensorData.dateTime.InitWithEpoch32Time(sqlite3_column_int64(res, 1));
            sensorData.temperature = sqlite3_column_int64(res, 2);
            sensorData.humidity = sqlite3_column_double(res, 3);
            sensorData.pressure = sqlite3_column_double(res, 4);
            sensorData.sensors[0] = sqlite3_column_double(res, 5);
            sensorData.sensors[1] = sqlite3_column_double(res, 6);
            sensorData.sensors[2] = sqlite3_column_double(res, 7);
            sensorData.sensors[3] = sqlite3_column_double(res, 8);
            callback(sensorData);
            ++recordCount;
        }
        sqlite3_finalize(res);

        log_d("recordCount = %u",recordCount);
        log_d("end");
        return recordCount;
    }
} // namespace Database