#include <Arduino.h>

#include <FS.h>
#include <cstdlib>
#include <esp_log.h>
#include <sqlite3.h>
#include <future>
#include <thread>
#include <SD.h>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "Utils.hpp"
#include "Infos.hpp"

namespace Database
{
    static sqlite3* db{};

    auto SensorData::serialize( ArduinoJson::JsonVariant& json ) const -> void
    {
        json["id"] = this->id;
        json["datetime"] = Utils::DateTime::toString( std::chrono::system_clock::from_time_t( this->dateTime ) );
        json["temperature"] = this->temperature;
        json["humidity"] = this->humidity;
        json["pressure"] = this->pressure;
        for ( auto n : this->sensors )
        {
            json["sensors"].add( n );
        }
    }

    auto SensorData::get() -> SensorData
    {
        return
        {
            0,
            std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() ),
            Infos::getTemperature(),
            Infos::getHumidity(),
            Infos::getPressure(),
            { Infos::getSensor( 0 ), Infos::getSensor( 1 ), Infos::getSensor( 2 ) }
        };
    }

    static auto initializeDatabase() -> void
    {
        log_d( "begin" );

        sqlite3_initialize();

        const auto rc{sqlite3_open( "/sd/sensors_data.db", &db )};
        if ( rc != SQLITE_OK )
        {
            log_e( "database open error: %s\n", sqlite3_errmsg( db ) );
            std::abort();
        };

        log_d( "end" );
    }

    static auto createTable() -> void
    {
        log_d( "begin" );
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
                             "        SENSOR_3    NUMERIC              "
                             "    )                                    "};

            const auto rc{sqlite3_exec( db, query, nullptr, nullptr, nullptr )};
            if ( rc != SQLITE_OK )
            {
                log_e( "table create error: %s\n", sqlite3_errmsg( db ) );
                std::abort();
            }
        }
        {
            const auto query{"CREATE UNIQUE INDEX IF NOT EXISTS DATE_TIME_INDEX "
                             "ON SENSORS_DATA( DATE_TIME )                      "};

            const auto rc{sqlite3_exec( db, query, nullptr, nullptr, nullptr )};
            if ( rc != SQLITE_OK )
            {
                log_e( "table index error: %s\n", sqlite3_errmsg( db ) );
                std::abort();
            }
        }
        log_d( "end" );
    }

    static auto insert( const SensorData& sensorData ) -> int64_t
    {
        int64_t id{};
        log_d( "begin" );

        const auto query
        {
            "INSERT INTO SENSORS_DATA ( "
            "    DATE_TIME,             "
            "    TEMPERATURE,           "
            "    HUMIDITY,              "
            "    PRESSURE,              "
            "    SENSOR_1,              "
            "    SENSOR_2,              "
            "    SENSOR_3               "
            ")                          "
            "VALUES                     "
            "    (?,?,?,?,?,?,?)        "};

        sqlite3_stmt* res;
        const auto rc{sqlite3_prepare_v2( db, query, strlen( query ), &res, nullptr )};
        if ( rc != SQLITE_OK )
        {
            log_d( "insert prepare error: %s", sqlite3_errmsg( db ) );
            //std::abort();
        }
        else
        {
            sqlite3_bind_int64( res, 1, sensorData.dateTime );
            sqlite3_bind_double( res, 2, sensorData.temperature );
            sqlite3_bind_double( res, 3, sensorData.humidity );
            sqlite3_bind_double( res, 4, sensorData.pressure );
            sqlite3_bind_double( res, 5, sensorData.sensors[0] );
            sqlite3_bind_double( res, 6, sensorData.sensors[1] );
            sqlite3_bind_double( res, 7, sensorData.sensors[2] );
            if ( sqlite3_step( res ) != SQLITE_DONE )
            {
                log_d( "insert error: %s", sqlite3_errmsg( db ) );
                //std::abort();
            }
            sqlite3_finalize( res );
            id = sqlite3_last_insert_rowid( db );
        }
        log_d( "end" );
        return id;
    }

    static auto generate() -> void
    {
        insert( SensorData::get() );
    }

    auto init() -> void
    {
        log_d( "begin" );

        initializeDatabase();
        createTable();

        log_d( "end" );
    }

    auto getData(
        std::function<void( const SensorData& )> callback,
        int64_t id,
        std::chrono::system_clock::time_point start,
        std::chrono::system_clock::time_point end
    ) -> void
    {
        log_d( "begin" );

        const auto query
        {
            "SELECT                                       "
            "    ID,                                      "
            "    DATE_TIME,                               "
            "    TEMPERATURE,                             "
            "    HUMIDITY,                                "
            "    PRESSURE,                                "
            "    SENSOR_1,                                "
            "    SENSOR_2,                                "
            "    SENSOR_3                                 "
            "FROM                                         "
            "    SENSORS_DATA                             "
            "WHERE                                        "
            "        ( ID >= IFNULL(?,ID) )               "
            "    AND ( DATE_TIME >= IFNULL(?,DATE_TIME) ) "
            "    AND ( DATE_TIME <= IFNULL(?,DATE_TIME) ) "
            "LIMIT 20                                     "};

        sqlite3_stmt* res;
        const auto rc{sqlite3_prepare_v2( db, query, strlen( query ), &res, nullptr )};
        if ( rc != SQLITE_OK )
        {
            log_e( "select prepare error: %s", sqlite3_errmsg( db ) );
            std::abort();
        }

        if ( id != int64_t{} )
        {
            sqlite3_bind_int64( res, 1, id );
        }
        if ( start != std::chrono::system_clock::time_point::min() )
        {
            sqlite3_bind_int64( res, 2, std::chrono::system_clock::to_time_t( start ) );
        }
        if ( end != std::chrono::system_clock::time_point::max() )
        {
            sqlite3_bind_int64( res, 3, std::chrono::system_clock::to_time_t( end ) );
        }

        while ( sqlite3_step( res ) == SQLITE_ROW )
        {
            SensorData sensorData;
            sensorData.id = sqlite3_column_int64( res, 0 );
            sensorData.dateTime = sqlite3_column_int64( res, 1 );
            sensorData.temperature = sqlite3_column_double( res, 2 );
            sensorData.humidity = sqlite3_column_double( res, 3 );
            sensorData.pressure = sqlite3_column_double( res, 4 );
            sensorData.sensors[0] = sqlite3_column_double( res, 5 );
            sensorData.sensors[1] = sqlite3_column_double( res, 6 );
            sensorData.sensors[2] = sqlite3_column_double( res, 7 );
            callback( sensorData );
        }
        sqlite3_finalize( res );

        log_d( "end" );
    }

    auto process() -> void
    {
        Utils::bound( std::chrono::minutes( 5 ), Database::generate );
    }
} // namespace Database