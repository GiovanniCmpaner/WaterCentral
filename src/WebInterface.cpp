#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <cstdlib>
#include <esp_log.h>
#include <memory>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "WebInterface.hpp"
#include "RealTime.hpp"

namespace WebInterface
{
    static auto server{std::unique_ptr<AsyncWebServer>{}};

    static auto getConfiguration(AsyncWebServerRequest *request) -> void
    {
        auto response{new AsyncJsonResponse{}};
        auto responseJson{response->getRoot()};

        Configuration::serialize(responseJson, cfg);

        response->setLength();
        request->send(response);
    }

    static auto setConfiguration(AsyncWebServerRequest *request, JsonVariant &requestJson) -> void
    {
    }

    static auto getDateTime(AsyncWebServerRequest *request) -> void
    {
        auto response{new AsyncJsonResponse{}};
        auto responseJson{response->getRoot()};

        const auto str{RealTime::dateTimeToString(RealTime::now())};
        responseJson["date_time"] = str;

        response->setLength();
        request->send(response);
    }

    static auto setDateTime(AsyncWebServerRequest *request, JsonVariant &requestJson) -> void
    {
        auto requestObj{ requestJson.as<JsonObject>() };

        const auto dateTime{RealTime::stringToDateTime(requestObj["date_time"])};
        RealTime::adjust(dateTime);

        request->send(204);
    }

    static auto getSensorsData(AsyncWebServerRequest *request) -> void
    {
        auto response{new AsyncJsonResponse{}};
        auto responseJson{response->getRoot()};

        auto id{int64_t{}};
    	auto start{RtcDateTime{}};
        auto end{RtcDateTime{}};

        if(request->hasParam("id"))
        {
            id = request->getParam("id")->value().toInt();
        }
        if(request->hasParam("start"))
        {
            //start = request->getParam("start")->value();
        }
        if(request->hasParam("end"))
        {
            //end = request->getParam("end")->value();
        }

        log_d("id = %ld",id);
        log_d("start = %04u-%02u-%02u %02u:%02u:%02u", start.Year(), start.Month(), start.Day(), start.Hour(), start.Minute(), start.Second());
        log_d("end = %04u-%02u-%02u %02u:%02u:%02u", end.Year(), end.Month(), end.Day(), end.Hour(), end.Minute(), end.Second());

        Database::getSensorsData([&](const SensorData& sensorData){
            auto element{ArduinoJson::JsonVariant{}};
            SensorData::serialize(element,sensorData);
            responseJson.add(element);
        },id);

        response->setLength();
        request->send(response);
    }

    static auto configureServer() -> void
    {
        server.release();

        if (WiFi.getMode() == WIFI_MODE_STA)
        {
            server.reset(new AsyncWebServer{cfg.station.port});
        }
        else if (WiFi.getMode() == WIFI_MODE_AP)
        {
            server.reset(new AsyncWebServer{cfg.accessPoint.port});
        }

        if (server)
        {
            server->on("/configuration", HTTP_GET, getConfiguration);
            server->addHandler(new AsyncCallbackJsonWebHandler("/configuration",setConfiguration));

            server->on("/date_time", HTTP_GET, getDateTime);
            server->addHandler(new AsyncCallbackJsonWebHandler("/date_time",setDateTime));
            
            server->on("/sensors_data", HTTP_GET, getSensorsData);

            server->begin();
        }

        
    }

    static auto configureStation() -> void
    {
        log_d("begin");

        log_d("enabled = %u", cfg.station.enabled);
        log_d("mac = %02X-%02X-%02X-%02X-%02X-%02X", cfg.station.mac[0], cfg.station.mac[1], cfg.station.mac[2], cfg.station.mac[3], cfg.station.mac[4], cfg.station.mac[5]);
        log_d("ip = %u.%u.%u.%u", cfg.station.ip[0], cfg.station.ip[1], cfg.station.ip[2], cfg.station.ip[3]);
        log_d("netmask = %u.%u.%u.%u", cfg.station.netmask[0], cfg.station.netmask[1], cfg.station.netmask[2], cfg.station.netmask[3]);
        log_d("gateway = %u.%u.%u.%u", cfg.station.gateway[0], cfg.station.gateway[1], cfg.station.gateway[2], cfg.station.gateway[3]);
        log_d("port = %u", cfg.station.port);
        log_d("user = %s", cfg.station.user.data());
        log_d("password = %s", cfg.station.password.data());

        if (not cfg.station.enabled)
        {
            WiFi.mode(WIFI_MODE_NULL);
        }
        else
        {
            if (not WiFi.mode(WIFI_MODE_STA))
            {
                log_e("mode error");
                std::abort();
            }

            WiFi.persistent(false);
            WiFi.setAutoConnect(false);
            WiFi.setAutoReconnect(true);

            if (not WiFi.config(cfg.station.ip.data(), cfg.station.gateway.data(), cfg.station.netmask.data()))
            {
                log_e("config error");
                std::abort();
            }

            if (not WiFi.begin(cfg.station.user.data(), cfg.station.password.data()))
            {
                log_e("init error");
                std::abort();
            }
        }

        configureServer();

        log_d("end");
    }

    static auto configureAccessPoint() -> void
    {
        log_d("begin");

        log_d("enabled = %u", cfg.accessPoint.enabled);
        log_d("mac = %02X-%02X-%02X-%02X-%02X-%02X", cfg.accessPoint.mac[0], cfg.accessPoint.mac[1], cfg.accessPoint.mac[2], cfg.accessPoint.mac[3], cfg.accessPoint.mac[4], cfg.accessPoint.mac[5]);
        log_d("ip = %u.%u.%u.%u", cfg.accessPoint.ip[0], cfg.accessPoint.ip[1], cfg.accessPoint.ip[2], cfg.accessPoint.ip[3]);
        log_d("netmask = %u.%u.%u.%u", cfg.accessPoint.netmask[0], cfg.accessPoint.netmask[1], cfg.accessPoint.netmask[2], cfg.accessPoint.netmask[3]);
        log_d("gateway = %u.%u.%u.%u", cfg.accessPoint.gateway[0], cfg.accessPoint.gateway[1], cfg.accessPoint.gateway[2], cfg.accessPoint.gateway[3]);
        log_d("port = %u", cfg.accessPoint.port);
        log_d("user = %s", cfg.accessPoint.user.data());
        log_d("password = %s", cfg.accessPoint.password.data());
        log_d("duration = %u", cfg.accessPoint.duration);

        if (not cfg.accessPoint.enabled)
        {
            WiFi.mode(WIFI_MODE_NULL);
        }
        else
        {
            if (not WiFi.mode(WIFI_MODE_AP))
            {
                log_e("mode error");
                std::abort();
            }

            WiFi.persistent(false);

            if (not WiFi.softAPConfig(cfg.accessPoint.ip.data(), cfg.accessPoint.gateway.data(), cfg.accessPoint.netmask.data()))
            {
                log_e("config error");
                std::abort();
            }

            if (not WiFi.softAP(cfg.accessPoint.user.data(), cfg.accessPoint.password.data()))
            {
                log_e("init error");
                std::abort();
            }
        }

        configureServer();

        log_d("end");
    }

    auto init() -> void
    {
        log_d("begin");

        //configureAccessPoint();
        configureStation();

        log_d("end");
    }

    auto process() -> void
    {
    }
} // namespace WebInterface