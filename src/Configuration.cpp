#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <FS.h>
#include <FastCRC.h>
#include <SD.h>
#include <WiFi.h>
#include <cstdio>
#include <cstdlib>
#include <esp_log.h>

#include "Configuration.hpp"
#include "Peripherals.hpp"

static const auto defaultCfg{Configuration{
    {true,
     {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
     {192, 168, 1, 200},
     {255, 255, 255, 0},
     {192, 168, 1, 1},
     80,
     "WORKGROUP",
     "49WNN7F3CD@22"},
    {true,
     {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
     {192, 168, 1, 200},
     {255, 255, 255, 0},
     {192, 168, 1, 1},
     80,
     "WaterCentral",
     "W@t3rC3ntr4l",
     30},
    {true,
     {22, 0},
     {8, 0}}}};

static auto stationMAC{std::array<uint8_t, 6>{}};
static auto accessPointMAC{std::array<uint8_t, 6>{}};

auto Configuration::init() -> void
{
    log_d("begin");

    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.macAddress(stationMAC.data());
    WiFi.softAPmacAddress(accessPointMAC.data());
    WiFi.mode(WIFI_MODE_NULL);

    log_d("end");
}

auto Configuration::serialize(ArduinoJson::JsonVariant &json, const Configuration &cfg) -> void
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
}

auto Configuration::deserialize(const ArduinoJson::JsonVariant &json, Configuration &cfg) -> void
{
    {
        const auto accessPoint{json["access_point"]};
        {
            const auto enabled{accessPoint["enabled"]};
            if (not enabled.is<bool>())
            {
                cfg.accessPoint.enabled = defaultCfg.accessPoint.enabled;
            }
            else
            {
                cfg.accessPoint.enabled = enabled.as<bool>();
            }
        }
        cfg.accessPoint.enabled = accessPoint["enabled"].as<bool>();
        {
            cfg.accessPoint.mac = accessPointMAC;
        }
        {
            const auto ip{accessPoint["ip"]};
            if (not ip.is<ArduinoJson::JsonArray>() || ip.size() != cfg.accessPoint.ip.size())
            {
                cfg.accessPoint.ip = defaultCfg.accessPoint.ip;
            }
            else
            {
                for (auto i{0}; i < cfg.accessPoint.ip.size(); ++i)
                {
                    cfg.accessPoint.ip[i] = ip[i].as<uint8_t>();
                }
            }
        }
        {
            const auto netmask{accessPoint["netmask"]};
            if (not netmask.is<ArduinoJson::JsonArray>() || netmask.size() != cfg.accessPoint.netmask.size())
            {
                cfg.accessPoint.netmask = defaultCfg.accessPoint.netmask;
            }
            else
            {
                for (auto i{0}; i < cfg.accessPoint.netmask.size(); ++i)
                {
                    cfg.accessPoint.netmask[i] = netmask[i].as<uint8_t>();
                }
            }
        }
        {
            const auto gateway{accessPoint["gateway"]};
            if (not gateway.is<ArduinoJson::JsonArray>() || gateway.size() != cfg.accessPoint.gateway.size())
            {
                cfg.accessPoint.gateway = defaultCfg.accessPoint.gateway;
            }
            else
            {
                for (auto i{0}; i < cfg.accessPoint.gateway.size(); ++i)
                {
                    cfg.accessPoint.gateway[i] = gateway[i].as<uint8_t>();
                }
            }
        }
        {
            const auto port{accessPoint["port"]};
            if (not port.is<uint16_t>())
            {
                cfg.accessPoint.port = defaultCfg.accessPoint.port;
            }
            else
            {
                cfg.accessPoint.port = port.as<uint16_t>();
            }
        }
        {
            const auto user{accessPoint["user"]};
            if (not user.is<std::string>())
            {
                cfg.accessPoint.user = defaultCfg.accessPoint.user;
            }
            else
            {
                cfg.accessPoint.user = user.as<std::string>();
            }
        }
        {
            const auto password{accessPoint["password"]};
            if (not password.is<std::string>())
            {
                cfg.accessPoint.password = defaultCfg.accessPoint.password;
            }
            else
            {
                cfg.accessPoint.password = password.as<std::string>();
            }
        }
        {
            const auto duration{accessPoint["duration"]};
            if (not duration.is<uint16_t>())
            {
                cfg.accessPoint.duration = defaultCfg.accessPoint.duration;
            }
            else
            {
                cfg.accessPoint.duration = duration.as<uint16_t>();
            }
        }
    }
    {
        const auto station{json["station"]};
        {
            const auto enabled{station["enabled"]};
            if (not enabled.is<bool>())
            {
                cfg.station.enabled = defaultCfg.station.enabled;
            }
            else
            {
                cfg.station.enabled = enabled.as<bool>();
            }
        }
        {
            cfg.station.mac = stationMAC;
        }
        {
            const auto ip{station["ip"]};
            if (not ip.is<ArduinoJson::JsonArray>() || ip.size() != cfg.station.ip.size())
            {
                cfg.station.ip = defaultCfg.station.ip;
            }
            else
            {
                for (auto i{0}; i < cfg.station.ip.size(); ++i)
                {
                    cfg.station.ip[i] = ip[i].as<uint8_t>();
                }
            }
        }
        {
            const auto netmask{station["netmask"]};
            if (not netmask.is<ArduinoJson::JsonArray>() || netmask.size() != cfg.station.netmask.size())
            {
                cfg.station.netmask = defaultCfg.station.netmask;
            }
            else
            {
                for (auto i{0}; i < cfg.station.netmask.size(); ++i)
                {
                    cfg.station.netmask[i] = netmask[i].as<uint8_t>();
                }
            }
        }
        {
            const auto gateway{station["gateway"]};
            if (not gateway.is<ArduinoJson::JsonArray>() || gateway.size() != cfg.station.gateway.size())
            {
                cfg.station.gateway = defaultCfg.station.gateway;
            }
            else
            {
                for (auto i{0}; i < cfg.station.gateway.size(); ++i)
                {
                    cfg.station.gateway[i] = gateway[i].as<uint8_t>();
                }
            }
        }
        {
            const auto port{station["port"]};
            if (not port.is<uint16_t>())
            {
                cfg.station.port = defaultCfg.station.port;
            }
            else
            {
                cfg.station.port = port.as<uint16_t>();
            }
        }
        {
            const auto user{station["user"]};
            if (not user.is<std::string>())
            {
                cfg.station.user = defaultCfg.station.user;
            }
            else
            {
                cfg.station.user = user.as<std::string>();
            }
        }
        {
            const auto password{station["password"]};
            if (not password.is<std::string>())
            {
                cfg.station.password = defaultCfg.station.password;
            }
            else
            {
                cfg.station.password = password.as<std::string>();
            }
        }
    }
    {
        const auto autoSleepWakeUp{json["auto_sleep_wakeup"]};

        cfg.autoSleepWakeUp.enabled = autoSleepWakeUp["enabled"].as<bool>();
        {
            const auto sleepTime{autoSleepWakeUp["sleep_time"]};
            if (not sleepTime.is<ArduinoJson::JsonArray>() || sleepTime.size() != cfg.autoSleepWakeUp.sleepTime.size())
            {
                cfg.autoSleepWakeUp.sleepTime = defaultCfg.autoSleepWakeUp.sleepTime;
            }
            else
            {
                for (auto i{0}; i < cfg.autoSleepWakeUp.sleepTime.size(); ++i)
                {
                    cfg.autoSleepWakeUp.sleepTime[i] = sleepTime[i].as<uint8_t>();
                }
            }
        }
        {
            const auto wakeUpTime{autoSleepWakeUp["wakeup_time"]};
            if (not wakeUpTime.is<ArduinoJson::JsonArray>() || wakeUpTime.size() != cfg.autoSleepWakeUp.wakeUpTime.size())
            {
                cfg.autoSleepWakeUp.wakeUpTime = defaultCfg.autoSleepWakeUp.wakeUpTime;
            }
            else
            {
                for (auto i{0}; i < cfg.autoSleepWakeUp.wakeUpTime.size(); ++i)
                {
                    cfg.autoSleepWakeUp.wakeUpTime[i] = wakeUpTime[i].as<uint8_t>();
                }
            }
        }
    }
}

auto Configuration::load(Configuration *cfg) -> void
{
    log_d("begin");

    auto valid{true};

    if (not SD.exists("/configuration.json"))
    {
        log_d("file not found");
        valid = false;
    }
    else
    {
        auto file{SD.open("/configuration.json", FILE_READ)};
        file.setTimeout(3000);
        if (not file)
        {
            log_e("file error");
            valid = false;
        }
        else
        {
            auto doc{ArduinoJson::DynamicJsonDocument{2048}};
            auto err{ArduinoJson::deserializeJson(doc, file)};
            file.close();

            if (err != ArduinoJson::DeserializationError::Ok)
            {
                log_d("json error = %s", err.c_str());
                valid = false;
            }
            else
            {
                auto json{doc.as<ArduinoJson::JsonVariant>()};
                Configuration::deserialize(json, *cfg);
                //
                //auto fileHash{json["hash"].as<uint32_t>()};
                //auto calcHash{cfg->hash()};
                //if (fileHash != calcHash)
                //{
                //    log_d("hash mismatch");
                //    valid = false;
                //}
            }
        }
    }

    if (not valid)
    {
        *cfg = defaultCfg;
    }

    Configuration::save(*cfg);

    log_d("end");
}

auto Configuration::save(const Configuration &cfg) -> void
{
    log_d("begin");

    auto file{SD.open("/configuration.json", FILE_WRITE)};
    file.setTimeout(3000);
    if (not file)
    {
        log_e("file error");
        std::abort();
    }

    auto doc{ArduinoJson::DynamicJsonDocument{2048}};
    auto json{doc.as<ArduinoJson::JsonVariant>()};

    Configuration::serialize(json, cfg);
    //json["hash"] = cfg.hash();

    ArduinoJson::serializeJsonPretty(doc, file);
    file.close();

    log_d("end");
}

//auto Configuration::hash() const -> uint32_t
//{
//    auto hasher{FastCRC32{}};
//
//    auto hash{hasher.crc32(nullptr, 0)};
//
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->station.enabled), sizeof(this->station.enabled));
//    hash = hasher.crc32_upd(this->station.mac.data(), this->station.mac.size());
//    hash = hasher.crc32_upd(this->station.ip.data(), this->station.ip.size());
//    hash = hasher.crc32_upd(this->station.netmask.data(), this->station.netmask.size());
//    hash = hasher.crc32_upd(this->station.gateway.data(), this->station.gateway.size());
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->station.port), sizeof(this->station.port));
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(this->station.user.data()), this->station.user.size());
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(this->station.password.data()), this->station.password.size());
//
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->accessPoint.enabled), sizeof(this->accessPoint.enabled));
//    hash = hasher.crc32_upd(this->accessPoint.mac.data(), this->accessPoint.mac.size());
//    hash = hasher.crc32_upd(this->accessPoint.ip.data(), this->accessPoint.ip.size());
//    hash = hasher.crc32_upd(this->accessPoint.netmask.data(), this->accessPoint.netmask.size());
//    hash = hasher.crc32_upd(this->accessPoint.gateway.data(), this->accessPoint.gateway.size());
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->accessPoint.port), sizeof(this->accessPoint.port));
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(this->accessPoint.user.data()), this->accessPoint.user.size());
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(this->accessPoint.password.data()), this->accessPoint.password.size());
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->accessPoint.duration), sizeof(this->accessPoint.duration));
//
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->autoSleepWakeUp.enabled), sizeof(this->autoSleepWakeUp.enabled));
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->autoSleepWakeUp.sleep.hour), sizeof(this->autoSleepWakeUp.sleep.hour));
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->autoSleepWakeUp.sleep.minute), sizeof(this->autoSleepWakeUp.sleep.minute));
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->autoSleepWakeUp.sleep.second), sizeof(this->autoSleepWakeUp.sleep.second));
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->autoSleepWakeUp.wakeUp.hour), sizeof(this->autoSleepWakeUp.wakeUp.hour));
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->autoSleepWakeUp.wakeUp.minute), sizeof(this->autoSleepWakeUp.wakeUp.minute));
//    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->autoSleepWakeUp.wakeUp.second), sizeof(this->autoSleepWakeUp.wakeUp.second));
//
//    return hash;
//}

Configuration cfg{};