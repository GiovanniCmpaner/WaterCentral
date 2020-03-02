#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <FS.h>
#include <FastCRC.h>
#include <SD.h>
#include <WiFi.h>
#include <cstdio>
#include <cstdlib>
#include <esp_log.h>

#include <string>
//#undef B1
//#include <nlohmannJson.hpp>

#include "Configuration.hpp"
#include "Peripherals.hpp"

static const Configuration defaultCfg
{
    {
        true,
        {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
        {192, 168, 1, 200},
        {255, 255, 255, 0},
        {192, 168, 1, 1},
        80,
        "WORKGROUP",
        "49WNN7F3CD@22"
    },
    {
        true,
        {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
        {192, 168, 1, 200},
        {255, 255, 255, 0},
        {192, 168, 1, 1},
        80,
        "WaterCentral",
        "W@t3rC3ntr4l",
        30
    },
    {
        true,
        {22, 0},
        {8, 0}
    },
    {
        {
            {
                true,
                "Ext",
                Configuration::Sensor::Type::MPX5500,
                0.0,
                395.6,
                {
                    +0.00115189,
                        -0.10866961
                    },
                {
                    true,
                    25.0
                }
            },
            {
                true,
                "Sup",
                Configuration::Sensor::Type::MPX5100,
                49.18,
                52.55,
                {
                    +0.00115442,
                        -0.10876640
                    },
                {
                    true,
                    40.0
                }
            },
            {
                true,
                "Inf",
                Configuration::Sensor::Type::MPX5100,
                17.9,
                22.98,
                {
                    0.00110177,
                    -0.25609925
                },
                {
                    true,
                    25.0
                }
            }
        }
    }
};

static std::array<uint8_t, 6> stationMAC{};
static std::array<uint8_t, 6> accessPointMAC{};

auto Configuration::init() -> void
{
    log_d( "begin" );

    WiFi.mode( WIFI_MODE_APSTA );
    WiFi.macAddress( stationMAC.data() );
    WiFi.softAPmacAddress( accessPointMAC.data() );
    WiFi.mode( WIFI_MODE_NULL );

    log_d( "end" );
}

auto Configuration::serialize( ArduinoJson::JsonVariant& json ) const -> void
{
    {
        auto accessPoint{json["access_point"]};

        accessPoint["enabled"] = this->accessPoint.enabled;
        for ( auto n : accessPointMAC )
        {
            accessPoint["mac"].add( n );
        }
        for ( auto n : this->accessPoint.ip )
        {
            accessPoint["ip"].add( n );
        }
        for ( auto n : this->accessPoint.netmask )
        {
            accessPoint["netmask"].add( n );
        }
        for ( auto n : this->accessPoint.gateway )
        {
            accessPoint["gateway"].add( n );
        }
        accessPoint["port"] = this->accessPoint.port;
        accessPoint["user"] = this->accessPoint.user;
        accessPoint["password"] = this->accessPoint.password;
        accessPoint["duration"] = this->accessPoint.duration;
    }
    {
        auto station{json["station"]};

        station["enabled"] = this->station.enabled;
        for ( auto n : stationMAC )
        {
            station["mac"].add( n );
        }
        for ( auto n : this->station.ip )
        {
            station["ip"].add( n );
        }
        for ( auto n : this->station.netmask )
        {
            station["netmask"].add( n );
        }
        for ( auto n : this->station.gateway )
        {
            station["gateway"].add( n );
        }
        station["port"] = this->station.port;
        station["user"] = this->station.user;
        station["password"] = this->station.password;
    }
    {
        auto autoSleepWakeUp{json["auto_sleep_wakeup"]};

        autoSleepWakeUp["enabled"] = this->autoSleepWakeUp.enabled;
        for ( auto n : this->autoSleepWakeUp.sleepTime )
        {
            autoSleepWakeUp["sleep_time"].add( n );
        }
        for ( auto n : this->autoSleepWakeUp.wakeUpTime )
        {
            autoSleepWakeUp["wakeup_time"].add( n );
        }
    }
    {
        auto sensors{json["sensors"]};

        for( auto& s : this->sensors )
        {
            auto sensor{ sensors.addElement() };

            sensor["enabled"] = s.enabled;
            sensor["name"] = s.name;
            sensor["type"] = static_cast<int16_t>( s.type );
            sensor["min"] = s.min;
            sensor["max"] = s.max;
            {
                auto calibration{ sensor["calibration"] };

                calibration["factor"] = s.calibration.angularCoefficient;
                calibration["offset"] = s.calibration.linearCoefficient;
            }
            {
                auto alarm{ sensor["alarm"] };

                alarm["enabled"] = s.alarm.enabled;
                alarm["value"] = s.alarm.value;
            }
        }
    }
}

auto Configuration::deserialize( const ArduinoJson::JsonVariant& json ) -> void
{
    {
        const auto accessPoint{json["access_point"]};
        {
            const auto enabled{accessPoint["enabled"]};
            if ( enabled.is<bool>() )
            {
                this->accessPoint.enabled = enabled.as<bool>();
            }
        }
        {
            this->accessPoint.mac = accessPointMAC;
        }
        {
            const auto ip{accessPoint["ip"]};
            if ( ip.is<ArduinoJson::JsonArray>() and ip.size() == this->accessPoint.ip.size() )
            {
                for ( auto i{0}; i < this->accessPoint.ip.size(); ++i )
                {
                    this->accessPoint.ip[i] = ip[i].as<uint8_t>();
                }
            }
        }
        {
            const auto netmask{accessPoint["netmask"]};
            if ( netmask.is<ArduinoJson::JsonArray>() and netmask.size() == this->accessPoint.netmask.size() )
            {
                for ( auto i{0}; i < this->accessPoint.netmask.size(); ++i )
                {
                    this->accessPoint.netmask[i] = netmask[i].as<uint8_t>();
                }
            }
        }
        {
            const auto gateway{accessPoint["gateway"]};
            if ( gateway.is<ArduinoJson::JsonArray>() and gateway.size() == this->accessPoint.gateway.size() )
            {
                for ( auto i{0}; i < this->accessPoint.gateway.size(); ++i )
                {
                    this->accessPoint.gateway[i] = gateway[i].as<uint8_t>();
                }
            }
        }
        {
            const auto port{accessPoint["port"]};
            if ( port.is<uint16_t>() )
            {
                this->accessPoint.port = port.as<uint16_t>();
            }
        }
        {
            const auto user{accessPoint["user"]};
            if ( user.is<std::string>() )
            {
                this->accessPoint.user = user.as<std::string>();
            }
        }
        {
            const auto password{accessPoint["password"]};
            if ( password.is<std::string>() )
            {
                this->accessPoint.password = password.as<std::string>();
            }
        }
        {
            const auto duration{accessPoint["duration"]};
            if ( duration.is<uint16_t>() )
            {
                log_d( "duration = %u -> %u", this->accessPoint.duration, duration.as<uint16_t>() );
                this->accessPoint.duration = duration.as<uint16_t>();
            }
            else
            {
                log_d( "duration error" );
            }
            log_d( "after = %u", this->accessPoint.duration );
        }
    }
    {
        const auto station{json["station"]};
        {
            const auto enabled{station["enabled"]};
            if ( enabled.is<bool>() )
            {
                this->station.enabled = enabled.as<bool>();
            }
        }
        {
            this->station.mac = stationMAC;
        }
        {
            const auto ip{station["ip"]};
            if ( ip.is<ArduinoJson::JsonArray>() and ip.size() == this->station.ip.size() )
            {
                for ( auto i{0}; i < this->station.ip.size(); ++i )
                {
                    this->station.ip[i] = ip[i].as<uint8_t>();
                }
            }
        }
        {
            const auto netmask{station["netmask"]};
            if ( netmask.is<ArduinoJson::JsonArray>() and netmask.size() == this->station.netmask.size() )
            {
                for ( auto i{0}; i < this->station.netmask.size(); ++i )
                {
                    this->station.netmask[i] = netmask[i].as<uint8_t>();
                }
            }
        }
        {
            const auto gateway{station["gateway"]};
            if ( gateway.is<ArduinoJson::JsonArray>() and gateway.size() == this->station.gateway.size() )
            {
                for ( auto i{0}; i < this->station.gateway.size(); ++i )
                {
                    this->station.gateway[i] = gateway[i].as<uint8_t>();
                }
            }
        }
        {
            const auto port{station["port"]};
            if ( port.is<uint16_t>() )
            {
                this->station.port = port.as<uint16_t>();
            }
        }
        {
            const auto user{station["user"]};
            if ( user.is<std::string>() )
            {
                this->station.user = user.as<std::string>();
            }
        }
        {
            const auto password{station["password"]};
            if ( password.is<std::string>() )
            {
                this->station.password = password.as<std::string>();
            }
        }
    }
    {
        const auto autoSleepWakeUp{json["auto_sleep_wakeup"]};
        {
            const auto enabled{autoSleepWakeUp["enabled"]};
            if ( enabled.is<bool>() )
            {
                this->autoSleepWakeUp.enabled = enabled.as<bool>();
            }
        }
        {
            const auto sleepTime{autoSleepWakeUp["sleep_time"]};
            if ( sleepTime.is<ArduinoJson::JsonArray>() and sleepTime.size() == this->autoSleepWakeUp.sleepTime.size() )
            {
                for ( auto i{0}; i < this->autoSleepWakeUp.sleepTime.size(); ++i )
                {
                    this->autoSleepWakeUp.sleepTime[i] = sleepTime[i].as<uint8_t>();
                }
            }
        }
        {
            const auto wakeUpTime{autoSleepWakeUp["wakeup_time"]};
            if ( wakeUpTime.is<ArduinoJson::JsonArray>() and wakeUpTime.size() == this->autoSleepWakeUp.wakeUpTime.size() )
            {
                for ( auto i{0}; i < this->autoSleepWakeUp.wakeUpTime.size(); ++i )
                {
                    this->autoSleepWakeUp.wakeUpTime[i] = wakeUpTime[i].as<uint8_t>();
                }
            }
        }
    }
    {
        const auto sensors{json["sensors"]};
        if ( sensors.is<ArduinoJson::JsonArray>() and sensors.size() == this->sensors.size() )
        {
            for ( auto i{0}; i < this->sensors.size(); ++i )
            {
                const auto sensor{sensors[i]};
                {
                    const auto enabled{ sensor["enabled"] };
                    if( enabled.is<bool>() )
                    {
                        this->sensors[i].enabled = enabled.as<bool>();
                    }
                }
                {
                    const auto name{ sensor["name"] };
                    if( name.is<std::string>() )
                    {
                        this->sensors[i].name = name.as<std::string>();
                    }
                }
                {
                    const auto type{ sensor["type"] };
                    if( type.is<int16_t>() )
                    {
                        this->sensors[i].type = static_cast<Configuration::Sensor::Type>( type.as<int16_t>() );
                    }
                }
                {
                    const auto min{ sensor["min"] };
                    if( min.is<double>() )
                    {
                        this->sensors[i].min = min.as<double>();
                    }
                }
                {
                    const auto max{ sensor["max"] };
                    if( max.is<double>() )
                    {
                        this->sensors[i].max = max.as<double>();
                    }
                }
                {
                    const auto calibration{ sensor["calibration"] };
                    {
                        const auto factor{ calibration["factor"] };
                        if( factor.is<double>() )
                        {
                            this->sensors[i].calibration.angularCoefficient = factor.as<double>();
                        }
                    }
                    {
                        const auto offset{ calibration["offset"] };
                        if( offset.is<double>() )
                        {
                            this->sensors[i].calibration.linearCoefficient = offset.as<double>();
                        }
                    }
                }
                {
                    const auto alarm{ sensor["alarm"] };
                    {
                        const auto enabled{ alarm["enabled"] };
                        if( enabled.is<bool>() )
                        {
                            this->sensors[i].alarm.enabled = enabled.as<bool>();
                        }
                    }
                    {
                        const auto value{ alarm["value"] };
                        if( value.is<double>() )
                        {
                            this->sensors[i].alarm.value = value.as<double>();
                        }
                    }
                }
            }
        }
    }
}

auto Configuration::load( Configuration* cfg ) -> void
{
    log_d( "begin" );

    *cfg = defaultCfg;

    if ( not SD.exists( "/configuration.json" ) )
    {
        log_d( "file not found" );
    }
    else
    {
        auto file{SD.open( "/configuration.json", FILE_READ )};
        file.setTimeout( 3000 );
        if ( not file )
        {
            log_e( "file error" );
        }
        else
        {
            auto doc{ArduinoJson::DynamicJsonDocument{3072}};
            auto err{ArduinoJson::deserializeJson( doc, file )};
            file.close();

            if ( err != ArduinoJson::DeserializationError::Ok )
            {
                log_d( "json error = %s", err.c_str() );
            }
            else
            {
                auto json{doc.as<ArduinoJson::JsonVariant>()};
                cfg->deserialize( json );
            }
        }
    }

    Configuration::save( *cfg );

    log_d( "end" );
}

auto Configuration::save( const Configuration& cfg ) -> void
{
    log_d( "begin" );

    auto file{SD.open( "/configuration.json", FILE_WRITE )};
    file.setTimeout( 3000 );
    if ( not file )
    {
        log_e( "file error" );
        std::abort();
    }

    auto doc{ArduinoJson::DynamicJsonDocument{3072}};
    auto json{doc.as<ArduinoJson::JsonVariant>()};

    cfg.serialize( json );

    ArduinoJson::serializeJsonPretty( doc, file );
    file.close();

    log_d( "end" );
}

Configuration cfg{};