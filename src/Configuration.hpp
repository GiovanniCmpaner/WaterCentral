#pragma once

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <array>

struct Configuration
{
    struct Station
    {
        std::array<uint8_t, 6> mac;
        std::array<uint8_t, 4> ip;
        std::array<uint8_t, 4> netmask;
        std::array<uint8_t, 4> gateway;
        uint16_t port;
        std::string user;
        std::string password;
    } station;

    struct AccessPoint
    {
        std::array<uint8_t, 6> mac;
        std::array<uint8_t, 4> ip;
        std::array<uint8_t, 4> netmask;
        std::array<uint8_t, 4> gateway;
        uint16_t port;
        std::string user;
        std::string password;
        uint16_t duration;
    } accessPoint;

    static auto init() -> bool;
    auto load() -> bool;
    auto save() -> bool;
    auto hash() const -> uint32_t;

    static auto serialize(ArduinoJson::JsonDocument &doc, const Configuration &cfg) -> void;
    static auto deserialize(const ArduinoJson::JsonDocument &doc, Configuration &cfg) -> void;
};

extern Configuration cfg;