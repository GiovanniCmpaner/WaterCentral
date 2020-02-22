#include <Arduino.h>
#include <ArduinoJson.hpp>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <chrono>
#include <cstdlib>
#include <esp_log.h>
#include <functional>
#include <memory>

#include "Configuration.hpp"
#include "Database.hpp"
#include "Peripherals.hpp"
#include "RealTime.hpp"
#include "WebInterface.hpp"

extern const uint8_t jquery_min_js_start[] asm("_binary_html_jquery_min_js_start");
extern const uint8_t jquery_min_js_end[] asm("_binary_html_jquery_min_js_end");
extern const uint8_t configuration_html_start[] asm("_binary_html_configuration_html_start");
extern const uint8_t configuration_html_end[] asm("_binary_html_configuration_html_end");

namespace WebInterface
{
    static auto server
    {
        std::unique_ptr<AsyncWebServer> {}
    };

    namespace Get
    {
        static auto handleProgmem(AsyncWebServerRequest* request, const std::string& contentType, const uint8_t* content, size_t len) -> void
        {
            const auto lastModified{RealTime::compiledDateTime()};
            auto ifModifiedSince{std::chrono::system_clock::time_point::min()};

            if (request->hasHeader("If-Modified-Since"))
            {
                ifModifiedSince = RealTime::stringToDateTimeHttp(request->getHeader("If-Modified-Since")->value().c_str());
            }

            auto response{lastModified <= ifModifiedSince ? request->beginResponse(304) : request->beginResponse_P(200, contentType.data(), content, len)};
            response->addHeader("Last-Modified", RealTime::dateTimeToStringHttp(lastModified).data());
            response->addHeader("Date", RealTime::dateTimeToStringHttp(std::chrono::system_clock::now()).data());
            response->addHeader("Cache-Control", "public, max-age=0");
            request->send(response);
        }

        static auto handleConfigurationJson(AsyncWebServerRequest* request) -> void
        {
            auto response{new AsyncJsonResponse{false, 2048}};
            auto responseJson{response->getRoot()};

            Configuration::serialize(responseJson, cfg);

            response->setLength();
            request->send(response);
        }
        static auto handleSensorsDataJson(AsyncWebServerRequest* request) -> void
        {
            auto response{new AsyncJsonResponse{true, 4096}};
            auto responseJson{response->getRoot()};

            auto id{int64_t{}};
            auto start{std::chrono::system_clock::time_point::min()};
            auto end{std::chrono::system_clock::time_point::max()};

            if (request->hasParam("id"))
            {
                id = request->getParam("id")->value().toInt();
            }
            if (request->hasParam("start"))
            {
                start = RealTime::stringToDateTime(request->getParam("start")->value().c_str());
            }
            if (request->hasParam("end"))
            {
                end = RealTime::stringToDateTime(request->getParam("end")->value().c_str());
            }

            log_d("id = %ld", id);
            log_d("start = %s", RealTime::dateTimeToString(start).data());
            log_d("end = %s", RealTime::dateTimeToString(end).data());

            Database::getSensorsData([&](const Database::SensorData &sensorData)
            {
                auto element{responseJson.addElement()};
                Database::SensorData::serialize(element, sensorData);
            },id, start, end);

            response->setLength();
            request->send(response);
        }

        static auto handleConfigurationHtml(AsyncWebServerRequest* request) -> void
        {
            handleProgmem(request,"text/html", configuration_html_start, static_cast<size_t>(configuration_html_end - configuration_html_start));
        }

        static auto handleJqueryJs(AsyncWebServerRequest* request) -> void
        {
            handleProgmem(request,"application/javascript", jquery_min_js_start, static_cast<size_t>(jquery_min_js_end - jquery_min_js_start));
        }

        static auto handleDateTimeJson(AsyncWebServerRequest* request) -> void
        {
            auto response{new AsyncJsonResponse{}};
            auto responseJson{response->getRoot()};

            const auto str{RealTime::dateTimeToString(std::chrono::system_clock::now())};
            responseJson["date_time"] = str;

            response->setLength();
            request->send(response);
        }
    } // namespace Get

    namespace Post
    {
        static auto handleConfigurationJson(AsyncWebServerRequest* request, JsonVariant& requestJson) -> void
        {
            Configuration::deserialize(requestJson, cfg);
            Configuration::save(cfg);
            request->send(204);
        }

        static auto handleDateTimeJson(AsyncWebServerRequest* request, JsonVariant& requestJson) -> void
        {
            auto requestObj{requestJson.as<JsonObject>()};

            const auto dateTime{RealTime::stringToDateTime(requestObj["date_time"])};
            RealTime::adjustDateTime(dateTime);

            request->send(204);
        }
    } // namespace Post

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
            server->on("/configuration.json", HTTP_GET, Get::handleConfigurationJson);
            server->addHandler(new AsyncCallbackJsonWebHandler("/configuration.json", Post::handleConfigurationJson));

            server->on("/date_time.json", HTTP_GET, Get::handleDateTimeJson);
            server->addHandler(new AsyncCallbackJsonWebHandler("/date_time.json", Post::handleDateTimeJson));

            server->on("/sensors_data.json", HTTP_GET, Get::handleSensorsDataJson);

            server->on("/configuration.html", HTTP_GET, Get::handleConfigurationHtml);
            server->on("/jquery.min.js", HTTP_GET, Get::handleJqueryJs);

            DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
            DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
            DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
            DefaultHeaders::Instance().addHeader("Access-Control-Max-Age", "86400");
            server->onNotFound([](AsyncWebServerRequest *request)
            {
                if (request->method() == HTTP_OPTIONS)
                {
                    request->send(200);
                }
                else
                {
                    request->send(404);
                }
            });
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