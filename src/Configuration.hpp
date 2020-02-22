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
    };

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
    };

    struct AutoSleepWakeUp
    {
        bool enabled;
        std::array<uint8_t, 2> sleepTime;
        std::array<uint8_t, 2> wakeUpTime;
    };

    struct Sensor
    {
        enum Type
        {
            MPX5050,
            MPX5100,
            MPX5500,
            MPX5700
        };

        struct Alarm
        {
            bool enabled;
            double value;
        };

        struct Calibration
        {
            double factor;
            double offset;
        };

        bool enabled;
        std::string name;
        Type type;
        double min;
        double max;
        Calibration calibration;
        Alarm alarm;
    };

    Station station;
    AccessPoint accessPoint;
    AutoSleepWakeUp autoSleepWakeUp;
    std::array<Sensor, 3> sensors;

    static auto init() -> void;
    static auto load(Configuration *cfg) -> void;
    static auto save(const Configuration &cfg) -> void;

    static auto serialize(ArduinoJson::JsonVariant &json, const Configuration &cfg) -> void;
    static auto deserialize(const ArduinoJson::JsonVariant &json, Configuration &cfg) -> void;
};

extern Configuration cfg;