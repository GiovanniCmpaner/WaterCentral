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

    static auto createTable() -> void
    {
        log_d("begin");

        const auto query{"CREATE TABLE IF NOT EXISTS               "
                         "    sensors_data (                       "
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

        log_d("end");
    }

    static auto insertSensorData(const SensorData &sensorData) -> int64_t
    {
        log_d("begin");

        const auto query{
            "INSERT INTO sensors_data (               "
            "    ID,                                  "
            "    DATE_TIME,                           "
            "    TEMPERATURE,                         "
            "    HUMIDITY,                            "
            "    PRESSURE,                            "
            "    SENSOR_1,                            "
            "    SENSOR_2,                            "
            "    SENSOR_3,                            "
            "    SENSOR_4                             "
            ")                                        "
            "VALUES                                   "
            "    (?,?,?,?,?,?,?,?,?)                  "};

        auto res{(sqlite3_stmt *){}};
        const auto rc{sqlite3_prepare_v2(db, query, 1000, &res, nullptr)};
        if (rc != SQLITE_OK)
        {
            log_e("insert prepare error: %s", sqlite3_errmsg(db));
            std::abort();
        }

        sqlite3_bind_int64(res, 0, sensorData.id);
        sqlite3_bind_int(res, 1, sensorData.dateTime.Epoch32Time());
        sqlite3_bind_double(res, 2, sensorData.temperature);
        sqlite3_bind_double(res, 3, sensorData.humidity);
        sqlite3_bind_double(res, 4, sensorData.pressure);
        sqlite3_bind_double(res, 5, sensorData.sensors[0]);
        sqlite3_bind_double(res, 6, sensorData.sensors[1]);
        sqlite3_bind_double(res, 7, sensorData.sensors[2]);
        sqlite3_bind_double(res, 8, sensorData.sensors[3]);

        if (sqlite3_step(res) != SQLITE_DONE)
        {
            log_e("insert error: %s", sqlite3_errmsg(db));
            std::abort();
        }

        sqlite3_finalize(res);

        log_d("end");
        return sqlite3_last_insert_rowid(db);
    }

    static auto process(void *) -> void
    {
        auto interval{pdMS_TO_TICKS(((RealTime::now() + 299) / 300) * 300 * 1000UL)};
        log_d("interval = %lu",interval);

        auto lastWakeTime{xTaskGetTickCount()};
        while (1)
        {
            vTaskDelayUntil(&lastWakeTime, interval);
            insertSensorData(SensorData::get());
            interval = 300 * 1000UL;
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
        xTaskCreate(process, "Database::process", 2048, nullptr, 0, &taskHandle);
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
            "    sensors_data                         "
            "WHERE                                    "
            "    ( ID >= IFNULL(?,ID) )               "
            "AND ( DATE_TIME >= IFNULL(?,DATE_TIME) ) "
            "AND ( DATE_TIME <= IFNULL(?,DATE_TIME) ) "};

        auto res{(sqlite3_stmt *){}};
        const auto rc{sqlite3_prepare_v2(db, query, 1000, &res, nullptr)};
        if (rc != SQLITE_OK)
        {
            log_e("select prepare error: %s", sqlite3_errmsg(db));
            std::abort();
        }

        if (id != 0)
        {
            sqlite3_bind_int64(res, 0, id);
        }
        if (start.IsValid())
        {
            sqlite3_bind_int(res, 1, start.Epoch32Time());
        }
        if (end.IsValid())
        {
            sqlite3_bind_int(res, 2, end.Epoch32Time());
        }

        while (sqlite3_step(res) == SQLITE_ROW)
        {
            auto sensorData{SensorData{}};
            sensorData.id = sqlite3_column_int64(res, 0);
            sensorData.dateTime.InitWithEpoch32Time(sqlite3_column_int(res, 1));
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

        log_d("end");
        return recordCount;
    }
} // namespace Database