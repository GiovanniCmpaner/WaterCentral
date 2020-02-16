#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <FS.h>
#include <FastCRC.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <esp_log.h>

#include "Configuration.hpp"
#include "Peripherals.hpp"

static const Configuration defaultCfg{
    {{0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
     {192, 168, 1, 200},
     {255, 255, 255, 0},
     {192, 168, 1, 1},
     80,
     "WORKGROUP",
     "49WNN7F3CD@22"},
    {{0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED},
     {192, 168, 1, 200},
     {255, 255, 255, 0},
     {192, 168, 1, 1},
     80,
     "WaterCentral",
     "W@t3rC3ntr4l",
     30}};

static auto stationMAC{std::array<uint8_t, 6>{}};
static auto accessPointMAC{std::array<uint8_t, 6>{}};

auto Configuration::init() -> bool
{
    log_d("begin");

    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.macAddress(stationMAC.data());
    WiFi.softAPmacAddress(accessPointMAC.data());

    if (not SD.begin(Peripherals::SD_CARD::SS, Peripherals::SD_CARD::hspi) || SD.cardType() == CARD_NONE)
    {
        log_e("sd error");
        return false;
    }

    log_d("end");
    return true;
}

auto Configuration::serialize(ArduinoJson::JsonDocument &doc, const Configuration &cfg) -> void
{
    {
        auto accessPoint{doc["access_point"]};
        {
            auto mac{accessPoint["mac"]};
            for (auto i{0}; i < cfg.accessPoint.mac.size(); ++i)
            {
                mac.add(cfg.accessPoint.mac[i]);
            }
        }
        {
            auto ip{accessPoint["ip"]};
            for (auto i{0}; i < cfg.accessPoint.ip.size(); ++i)
            {
                ip.add(cfg.accessPoint.ip[i]);
            }
        }
        {
            auto netmask{accessPoint["netmask"]};
            for (auto i{0}; i < cfg.accessPoint.netmask.size(); ++i)
            {
                netmask.add(cfg.accessPoint.netmask[i]);
            }
        }
        {
            auto gateway{accessPoint["gateway"]};
            for (auto i{0}; i < cfg.accessPoint.gateway.size(); ++i)
            {
                gateway.add(cfg.accessPoint.gateway[i]);
            }
        }
        {
            auto port{accessPoint["port"]};
            port = cfg.accessPoint.port;
        }
        {
            auto user{accessPoint["user"]};
            user = cfg.accessPoint.user;
        }
        {
            auto password{accessPoint["password"]};
            password = cfg.accessPoint.password;
        }
        {
            auto duration{accessPoint["duration"]};
            duration = cfg.accessPoint.duration;
        }
    }
    {
        auto station{doc["station"]};
        {
            auto mac{station["mac"]};
            for (auto i{0}; i < cfg.station.mac.size(); ++i)
            {
                mac.add(cfg.station.mac[i]);
            }
        }
        {
            auto ip{station["ip"]};
            for (auto i{0}; i < cfg.station.ip.size(); ++i)
            {
                ip.add(cfg.station.ip[i]);
            }
        }
        {
            auto netmask{station["netmask"]};
            for (auto i{0}; i < cfg.station.netmask.size(); ++i)
            {
                netmask.add(cfg.station.netmask[i]);
            }
        }
        {
            auto gateway{station["gateway"]};
            for (auto i{0}; i < cfg.station.gateway.size(); ++i)
            {
                gateway.add(cfg.station.gateway[i]);
            }
        }
        {
            auto port{station["port"]};
            port = cfg.station.port;
        }
        {
            auto user{station["user"]};
            user = cfg.station.user;
        }
        {
            auto password{station["password"]};
            password = cfg.station.password;
        }
    }
}

auto Configuration::deserialize(const ArduinoJson::JsonDocument &doc, Configuration &cfg) -> void
{
    {
        const auto accessPoint{doc["access_point"]};
        {
            const auto mac{accessPoint["mac"]};
            if (not mac.is<ArduinoJson::JsonArray>() || mac.size() != cfg.accessPoint.mac.size())
            {
                cfg.accessPoint.mac = defaultCfg.accessPoint.mac;
            }
            else
            {
                for (auto i{0}; i < cfg.accessPoint.mac.size(); ++i)
                {
                    cfg.accessPoint.mac[i] = mac[i].as<uint8_t>();
                }
            }
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
        const auto station{doc["station"]};
        {
            const auto mac{station["mac"]};
            if (not mac.is<ArduinoJson::JsonArray>() || mac.size() != cfg.station.mac.size())
            {
                cfg.station.mac = defaultCfg.station.mac;
            }
            else
            {
                for (auto i{0}; i < cfg.station.mac.size(); ++i)
                {
                    cfg.station.mac[i] = mac[i].as<uint8_t>();
                }
            }
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
}

auto Configuration::load() -> bool
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
                Configuration::deserialize(doc, *this);
                this->station.mac = stationMAC;
                this->accessPoint.mac = accessPointMAC;

                auto fileHash{doc["hash"].as<uint32_t>()};
                auto calcHash{this->hash()};
                if (fileHash != calcHash)
                {
                    log_d("hash mismatch");
                    valid = false;
                }
            }
        }
    }

    if (not valid)
    {
        *this = defaultCfg;
        if (not this->save())
        {
            log_e("save error");
            return false;
        }
    }

    log_d("end");
    return true;
}

auto Configuration::save() -> bool
{
    log_d("begin");

    auto file{SD.open("/configuration.json", FILE_WRITE)};
    file.setTimeout(3000);
    if (not file)
    {
        log_e("file error");
        return false;
    }

    auto doc{ArduinoJson::DynamicJsonDocument{2048}};

    this->station.mac = stationMAC;
    this->accessPoint.mac = accessPointMAC;
    Configuration::serialize(doc, *this);
    doc["hash"] = this->hash();

    ArduinoJson::serializeJsonPretty(doc, file);
    file.close();

    log_d("end");
    return true;
}

auto Configuration::hash() const -> uint32_t
{
    auto hasher{FastCRC32{}};

    auto hash{hasher.crc32(nullptr, 0)};

    hash = hasher.crc32_upd(this->station.mac.data(), this->station.mac.size());
    hash = hasher.crc32_upd(this->station.ip.data(), this->station.ip.size());
    hash = hasher.crc32_upd(this->station.netmask.data(), this->station.netmask.size());
    hash = hasher.crc32_upd(this->station.gateway.data(), this->station.gateway.size());
    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->station.port), sizeof(this->station.port));
    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(this->station.user.data()), this->station.user.size());
    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(this->station.password.data()), this->station.password.size());

    hash = hasher.crc32_upd(this->accessPoint.mac.data(), this->accessPoint.mac.size());
    hash = hasher.crc32_upd(this->accessPoint.ip.data(), this->accessPoint.ip.size());
    hash = hasher.crc32_upd(this->accessPoint.netmask.data(), this->accessPoint.netmask.size());
    hash = hasher.crc32_upd(this->accessPoint.gateway.data(), this->accessPoint.gateway.size());
    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->accessPoint.port), sizeof(this->accessPoint.port));
    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(this->accessPoint.user.data()), this->accessPoint.user.size());
    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(this->accessPoint.password.data()), this->accessPoint.password.size());
    hash = hasher.crc32_upd(reinterpret_cast<const uint8_t *>(&this->accessPoint.duration), sizeof(this->accessPoint.duration));

    return hash;
}

Configuration cfg{};