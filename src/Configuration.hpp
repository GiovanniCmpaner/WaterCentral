#pragma once

#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <array>

struct Configuration
{
    struct Station
    {
        bool enabled;
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
        bool enabled;
        std::array<uint8_t, 6> mac;
        std::array<uint8_t, 4> ip;
        std::array<uint8_t, 4> netmask;
        std::array<uint8_t, 4> gateway;
        uint16_t port;
        std::string user;
        std::string password;
        uint16_t duration;
    } accessPoint;

    struct AutoSleepWakeUp
    {
        bool enabled;
        struct
        {
            uint8_t hour;
            uint8_t minute;
            uint8_t second;
        } sleep, wakeUp;
    } autoSleepWakeUp;

    static auto init() -> void;
    static auto load(Configuration* cfg) -> void;
    static auto save(const Configuration& cfg) -> void;

    static auto serialize(ArduinoJson::JsonVariant &json, const Configuration &cfg) -> void;
    static auto deserialize(const ArduinoJson::JsonVariant &json, Configuration &cfg) -> void;
};

extern Configuration cfg;