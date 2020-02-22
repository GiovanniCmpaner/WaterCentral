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
                "Test1",
                Configuration::Sensor::Type::MPX5050,
                10.0,
                50.0,
                {
                    0.0,
                    1.0
                },
                {
                    true,
                    25.0
                }
            },
            {
                true,
                "Test2",
                Configuration::Sensor::Type::MPX5050,
                10.0,
                50.0,
                {
                    1.0,
                    0.0
                },
                {
                    true,
                    25.0
                }
            },
            {
                true,
                "Test3",
                Configuration::Sensor::Type::MPX5050,
                10.0,
                50.0,
                {
                    1.0,
                    0.0
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
    log_d("begin");

    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.macAddress(stationMAC.data());
    WiFi.softAPmacAddress(accessPointMAC.data());
    WiFi.mode(WIFI_MODE_NULL);

    log_d("end");
}

//void to_json(nlohmann::json& j, const Configuration::AccessPoint& accessPoint)
//{
//    j =
//    {
//        "enabled", cfg.accessPoint.enabled,
//        "mac", accessPointMAC,
//        "ip", cfg.accessPoint.ip,
//        "netmask", cfg.accessPoint.ip,
//        "gateway", cfg.accessPoint.gateway,
//        "port", cfg.accessPoint.port,
//        "user", cfg.accessPoint.user,
//        "password", cfg.accessPoint.password,
//        "duration", cfg.accessPoint.duration
//    };
//}
//
//
//void to_json(nlohmann::json& j, const Configuration& cfg)
//{
//    j =
//    {
//        {
//            "access_point", cfg.accessPoint
//        }
//    };
//}
//
//void from_json(const nlohmann::json& j, Configuration& cfg)
//{
//    j.at("name").get_to(p.name);
//    j.at("address").get_to(p.address);
//    j.at("age").get_to(p.age);
//}

auto Configuration::serialize(ArduinoJson::JsonVariant& json, const Configuration& cfg) -> void
{
    {
        auto accessPoint{json["access_point"]};

        accessPoint["enabled"] = cfg.accessPoint.enabled;
        for (auto n : accessPointMAC)
        {
            accessPoint["mac"].add(n);
        }
        for (auto n : cfg.accessPoint.ip)
        {
            accessPoint["ip"].add(n);
        }
        for (auto n : cfg.accessPoint.netmask)
        {
            accessPoint["netmask"].add(n);
        }
        for (auto n : cfg.accessPoint.gateway)
        {
            accessPoint["gateway"].add(n);
        }
        accessPoint["port"] = cfg.accessPoint.port;
        accessPoint["user"] = cfg.accessPoint.user;
        accessPoint["password"] = cfg.accessPoint.password;
        accessPoint["duration"] = cfg.accessPoint.duration;
    }
    {
        auto station{json["station"]};

        station["enabled"] = cfg.station.enabled;
        for (auto n : stationMAC)
        {
            station["mac"].add(n);
        }
        for (auto n : cfg.station.ip)
        {
            station["ip"].add(n);
        }
        for (auto n : cfg.station.netmask)
        {
            station["netmask"].add(n);
        }
        for (auto n : cfg.station.gateway)
        {
            station["gateway"].add(n);
        }
        station["port"] = cfg.station.port;
        station["user"] = cfg.station.user;
        station["password"] = cfg.station.password;
    }
    {
        auto autoSleepWakeUp{json["auto_sleep_wakeup"]};

        autoSleepWakeUp["enabled"] = cfg.autoSleepWakeUp.enabled;
        for (auto n : cfg.autoSleepWakeUp.sleepTime)
        {
            autoSleepWakeUp["sleep_time"].add(n);
        }
        for (auto n : cfg.autoSleepWakeUp.wakeUpTime)
        {
            autoSleepWakeUp["wakeup_time"].add(n);
        }
    }
    {
        auto sensors{json["sensors"]};

        for(auto& s : cfg.sensors)
        {
            auto sensor{ sensors.addElement() };

            sensor["enabled"] = s.enabled;
            sensor["name"] = s.name;
            sensor["type"] = static_cast<int16_t>(s.type);
            sensor["min"] = s.min;
            sensor["max"] = s.max;
            {
                auto calibration{ sensor["calibration"] };

                calibration["factor"] = s.calibration.factor;
                calibration["offset"] = s.calibration.offset;
            }
            {
                auto alarm{ sensor["alarm"] };

                alarm["enabled"] = s.alarm.enabled;
                alarm["value"] = s.alarm.value;
            }
        }
    }
}

auto Configuration::deserialize(const ArduinoJson::JsonVariant& json, Configuration& cfg) -> void
{
    {
        const auto accessPoint{json["access_point"]};
        {
            const auto enabled{accessPoint["enabled"]};
            if (enabled.is<bool>())
            {
                cfg.accessPoint.enabled = enabled.as<bool>();
            }
        }
        {
            cfg.accessPoint.mac = accessPointMAC;
        }
        {
            const auto ip{accessPoint["ip"]};
            if (ip.is<ArduinoJson::JsonArray>() and ip.size() == cfg.accessPoint.ip.size())
            {
                for (auto i{0}; i < cfg.accessPoint.ip.size(); ++i)
                {
                    cfg.accessPoint.ip[i] = ip[i].as<uint8_t>();
                }
            }
        }
        {
            const auto netmask{accessPoint["netmask"]};
            if (netmask.is<ArduinoJson::JsonArray>() and netmask.size() == cfg.accessPoint.netmask.size())
            {
                for (auto i{0}; i < cfg.accessPoint.netmask.size(); ++i)
                {
                    cfg.accessPoint.netmask[i] = netmask[i].as<uint8_t>();
                }
            }
        }
        {
            const auto gateway{accessPoint["gateway"]};
            if (gateway.is<ArduinoJson::JsonArray>() and gateway.size() == cfg.accessPoint.gateway.size())
            {
                for (auto i{0}; i < cfg.accessPoint.gateway.size(); ++i)
                {
                    cfg.accessPoint.gateway[i] = gateway[i].as<uint8_t>();
                }
            }
        }
        {
            const auto port{accessPoint["port"]};
            if (port.is<uint16_t>())
            {
                cfg.accessPoint.port = port.as<uint16_t>();
            }
        }
        {
            const auto user{accessPoint["user"]};
            if (user.is<std::string>())
            {
                cfg.accessPoint.user = user.as<std::string>();
            }
        }
        {
            const auto password{accessPoint["password"]};
            if (password.is<std::string>())
            {
                cfg.accessPoint.password = password.as<std::string>();
            }
        }
        {
            const auto duration{accessPoint["duration"]};
            if (duration.is<uint16_t>())
            {
                cfg.accessPoint.duration = duration.as<uint16_t>();
            }
        }
    }
    {
        const auto station{json["station"]};
        {
            const auto enabled{station["enabled"]};
            if (enabled.is<bool>())
            {
                cfg.station.enabled = enabled.as<bool>();
            }
        }
        {
            cfg.station.mac = stationMAC;
        }
        {
            const auto ip{station["ip"]};
            if (ip.is<ArduinoJson::JsonArray>() and ip.size() == cfg.station.ip.size())
            {
                for (auto i{0}; i < cfg.station.ip.size(); ++i)
                {
                    cfg.station.ip[i] = ip[i].as<uint8_t>();
                }
            }
        }
        {
            const auto netmask{station["netmask"]};
            if (netmask.is<ArduinoJson::JsonArray>() and netmask.size() == cfg.station.netmask.size())
            {
                for (auto i{0}; i < cfg.station.netmask.size(); ++i)
                {
                    cfg.station.netmask[i] = netmask[i].as<uint8_t>();
                }
            }
        }
        {
            const auto gateway{station["gateway"]};
            if (gateway.is<ArduinoJson::JsonArray>() and gateway.size() == cfg.station.gateway.size())
            {
                for (auto i{0}; i < cfg.station.gateway.size(); ++i)
                {
                    cfg.station.gateway[i] = gateway[i].as<uint8_t>();
                }
            }
        }
        {
            const auto port{station["port"]};
            if (port.is<uint16_t>())
            {
                cfg.station.port = port.as<uint16_t>();
            }
        }
        {
            const auto user{station["user"]};
            if (user.is<std::string>())
            {
                cfg.station.user = user.as<std::string>();
            }
        }
        {
            const auto password{station["password"]};
            if (password.is<std::string>())
            {
                cfg.station.password = password.as<std::string>();
            }
        }
    }
    {
        const auto autoSleepWakeUp{json["auto_sleep_wakeup"]};
        {
            const auto enabled{autoSleepWakeUp["enabled"]};
            if (enabled.is<bool>())
            {
                cfg.autoSleepWakeUp.enabled = enabled.as<bool>();
            }
        }
        {
            const auto sleepTime{autoSleepWakeUp["sleep_time"]};
            if (sleepTime.is<ArduinoJson::JsonArray>() and sleepTime.size() == cfg.autoSleepWakeUp.sleepTime.size())
            {
                for (auto i{0}; i < cfg.autoSleepWakeUp.sleepTime.size(); ++i)
                {
                    cfg.autoSleepWakeUp.sleepTime[i] = sleepTime[i].as<uint8_t>();
                }
            }
        }
        {
            const auto wakeUpTime{autoSleepWakeUp["wakeup_time"]};
            if (wakeUpTime.is<ArduinoJson::JsonArray>() and wakeUpTime.size() == cfg.autoSleepWakeUp.wakeUpTime.size())
            {
                for (auto i{0}; i < cfg.autoSleepWakeUp.wakeUpTime.size(); ++i)
                {
                    cfg.autoSleepWakeUp.wakeUpTime[i] = wakeUpTime[i].as<uint8_t>();
                }
            }
        }
    }
    {
        const auto sensors{json["sensors"]};

        for (auto i{0}; i < cfg.sensors.size(); ++i)
        {
            const auto sensor{sensors[i]};
            {
                const auto enabled{ sensor["enabled"] };
                if(enabled.is<bool>())
                {
                    cfg.sensors[i].enabled = enabled.as<bool>();
                }
            }
            {
                const auto name{ sensor["name"] };
                if(name.is<std::string>())
                {
                    cfg.sensors[i].name = name.as<std::string>();
                }
            }
            {
                const auto type{ sensor["type"] };
                if(type.is<int16_t>())
                {
                    cfg.sensors[i].type = static_cast<Configuration::Sensor::Type>(type.as<int16_t>());
                }
            }
            {
                const auto min{ sensor["min"] };
                if(min.is<double>())
                {
                    cfg.sensors[i].min = min.as<double>();
                }
            }
            {
                const auto max{ sensor["max"] };
                if(max.is<double>())
                {
                    cfg.sensors[i].max = max.as<double>();
                }
            }
            {
                const auto calibration{ sensor["calibration"] };
                {
                    const auto factor{ calibration["factor"] };
                    if(factor.is<double>())
                    {
                        cfg.sensors[i].calibration.factor = factor.as<double>();
                    }
                }
                {
                    const auto offset{ calibration["offset"] };
                    if(offset.is<double>())
                    {
                        cfg.sensors[i].calibration.offset = offset.as<double>();
                    }
                }
            }
            {
                const auto alarm{ sensor["alarm"] };
                {
                    const auto enabled{ alarm["enabled"] };
                    if(enabled.is<bool>())
                    {
                        cfg.sensors[i].alarm.enabled = enabled.as<bool>();
                    }
                }
                {
                    const auto value{ alarm["value"] };
                    if(value.is<double>())
                    {
                        cfg.sensors[i].alarm.value = value.as<double>();
                    }
                }
            }
        }
    }
}

auto Configuration::load(Configuration* cfg) -> void
{
    log_d("begin");

    *cfg = defaultCfg;

    if (not SD.exists("/configuration.json"))
    {
        log_d("file not found");
    }
    else
    {
        auto file{SD.open("/configuration.json", FILE_READ)};
        file.setTimeout(3000);
        if (not file)
        {
            log_e("file error");
        }
        else
        {
            auto doc{ArduinoJson::DynamicJsonDocument{3072}};
            auto err{ArduinoJson::deserializeJson(doc, file)};
            file.close();

            if (err != ArduinoJson::DeserializationError::Ok)
            {
                log_d("json error = %s", err.c_str());
            }
            else
            {
                auto json{doc.as<ArduinoJson::JsonVariant>()};
                Configuration::deserialize(json, *cfg);
            }
        }
    }

    Configuration::save(*cfg);

    log_d("end");
}

auto Configuration::save(const Configuration& cfg) -> void
{
    log_d("begin");

    auto file{SD.open("/configuration.json", FILE_WRITE)};
    file.setTimeout(3000);
    if (not file)
    {
        log_e("file error");
        std::abort();
    }

    auto doc{ArduinoJson::DynamicJsonDocument{3072}};
    auto json{doc.as<ArduinoJson::JsonVariant>()};

    Configuration::serialize(json, cfg);

    ArduinoJson::serializeJsonPretty(doc, file);
    file.close();

    log_d("end");
}

Configuration cfg{};