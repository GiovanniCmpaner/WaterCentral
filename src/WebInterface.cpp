#include <ArduinoJson.hpp>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <esp_log.h>

#include "Configuration.hpp"
#include "Peripherals.hpp"
#include "WebInterface.hpp"

namespace WebInterface
{
    static auto server{WiFiServer{}};
    static auto client{WiFiClient{}};

    auto setStation() -> bool
    {
        log_d("begin");

        log_d("mac = %02X-%02X-%02X-%02X-%02X-%02X", cfg.station.mac[0], cfg.station.mac[1], cfg.station.mac[2], cfg.station.mac[3], cfg.station.mac[4], cfg.station.mac[5]);
        log_d("ip = %u.%u.%u.%u", cfg.station.ip[0], cfg.station.ip[1], cfg.station.ip[2], cfg.station.ip[3]);
        log_d("netmask = %u.%u.%u.%u", cfg.station.netmask[0], cfg.station.netmask[1], cfg.station.netmask[2], cfg.station.netmask[3]);
        log_d("gateway = %u.%u.%u.%u", cfg.station.gateway[0], cfg.station.gateway[1], cfg.station.gateway[2], cfg.station.gateway[3]);
        log_d("port = %u", cfg.station.port);
        log_d("user = %s", cfg.station.user.data());
        log_d("password = %s", cfg.station.password.data());

        if (not WiFi.mode(WIFI_MODE_STA))
        {
            log_e("mode error");
            return false;
        }

        WiFi.persistent(false);
        WiFi.setAutoConnect(false);
        WiFi.setAutoReconnect(true);

        if (not WiFi.config(cfg.station.ip.data(), cfg.station.gateway.data(), cfg.station.netmask.data()))
        {
            log_e("config error");
            return false;
        }

        if (not WiFi.begin(cfg.station.user.data(), cfg.station.password.data()))
        {
            log_e("init error");
            return false;
        }

        client.stop();

        server.end();
        server = WiFiServer{cfg.station.port, 1};
        server.begin();

        log_d("end");
        return true;
    }

    auto setAccessPoint() -> bool
    {
        log_d("begin");

        log_d("mac = %02X-%02X-%02X-%02X-%02X-%02X", cfg.accessPoint.mac[0], cfg.accessPoint.mac[1], cfg.accessPoint.mac[2], cfg.accessPoint.mac[3], cfg.accessPoint.mac[4], cfg.accessPoint.mac[5]);
        log_d("ip = %u.%u.%u.%u", cfg.accessPoint.ip[0], cfg.accessPoint.ip[1], cfg.accessPoint.ip[2], cfg.accessPoint.ip[3]);
        log_d("netmask = %u.%u.%u.%u", cfg.accessPoint.netmask[0], cfg.accessPoint.netmask[1], cfg.accessPoint.netmask[2], cfg.accessPoint.netmask[3]);
        log_d("gateway = %u.%u.%u.%u", cfg.accessPoint.gateway[0], cfg.accessPoint.gateway[1], cfg.accessPoint.gateway[2], cfg.accessPoint.gateway[3]);
        log_d("port = %u", cfg.accessPoint.port);
        log_d("user = %s", cfg.accessPoint.user.data());
        log_d("password = %s", cfg.accessPoint.password.data());
        log_d("duration = %u", cfg.accessPoint.duration);

        if (not WiFi.mode(WIFI_MODE_AP))
        {
            log_e("mode error");
            return false;
        }

        WiFi.persistent(false);

        if (not WiFi.softAPConfig(cfg.accessPoint.ip.data(), cfg.accessPoint.gateway.data(), cfg.accessPoint.netmask.data()))
        {
            log_e("config error");
            return false;
        }

        if (not WiFi.softAP(cfg.accessPoint.user.data(), cfg.accessPoint.password.data()))
        {
            log_e("init error");
            return false;
        }

        client.stop();

        server.end();
        server = WiFiServer{cfg.accessPoint.port, 1};
        server.begin();

        log_d("end");
        return true;
    }

    auto init() -> bool
    {
        log_d("begin");

        setAccessPoint();

        log_d("end");
        return true;
    }
    auto process() -> void
    {
    }
} // namespace WebInterface