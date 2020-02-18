#include <Arduino.h>

#include <ArduinoJson.hpp>
#include <BME280.h>
#include <ResponsiveAnalogRead.h>

#include "Configuration.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include "SensorData.hpp"

auto SensorData::serialize(ArduinoJson::JsonVariant &json, const SensorData &sensorData) -> void
{
    json["id"] = sensorData.id;
    json["dateTime"] = RealTime::dateTimeToString(sensorData.dateTime);
    json["temperature"] = sensorData.temperature;
    json["humidity"] = sensorData.humidity;
    json["pressure"] = sensorData.pressure;
    for (auto n : sensorData.sensors)
    {
        json["sensors"].add(n);
    }
}

auto SensorData::get() -> SensorData
{
    auto sensorData{SensorData{}};
    sensorData.dateTime = RealTime::now();
    sensorData.temperature = 1.234;
    sensorData.humidity = 1.234;
    sensorData.pressure = 1.234;
    sensorData.sensors[0] = 1.234;
    sensorData.sensors[1] = 1.234;
    sensorData.sensors[2] = 1.234;
    sensorData.sensors[3] = 1.234;
    return sensorData;
}