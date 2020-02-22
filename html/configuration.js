$(document).ready(() => {
    getConfiguration();
    $("#configuration").submit((event) => {
        event.preventDefault();
        if ($("#configuration")[0].checkValidity()) {
            setConfiguration();
        }
    });
});

function setConfiguration() {
    var data = {
        access_point: {
            enabled: $("#access_point_enabled").prop("checked"),
            mac: $("#access_point_mac").prop("value").split("-").map((s) => parseInt(s, 16)),
            ip: $("#access_point_ip").prop("value").split(".").map((s) => parseInt(s, 10)),
            netmask: $("#access_point_netmask").prop("value").split(".").map((s) => parseInt(s, 10)),
            gateway: $("#access_point_gateway").prop("value").split(".").map((s) => parseInt(s, 10)),
            port: $("#access_point_port").prop("value"),
            user: $("#access_point_user").prop("value"),
            password: $("#access_point_password").prop("value"),
            duration: $("#access_point_duration").prop("value")
        },
        station: {
            enabled: $("#station_enabled").prop("checked"),
            mac: $("#station_mac").prop("value").split("-").map((s) => parseInt(s, 16)),
            ip: $("#station_ip").prop("value").split(".").map((s) => parseInt(s, 10)),
            netmask: $("#station_netmask").prop("value").split(".").map((s) => parseInt(s, 10)),
            gateway: $("#station_gateway").prop("value").split(".").map((s) => parseInt(s, 10)),
            port: $("#station_port").prop("value"),
            user: $("#station_user").prop("value"),
            password: $("#station_password").prop("value")
        },
        auto_sleep_wakeup: {
            enabled: $("#auto_sleep_wakeup_enabled").prop("checked"),
            sleep_time: $("#auto_sleep_wakeup_sleep_time").prop("value").split(":").map((s) => parseInt(s, 10)),
            wakeup_time: $("#auto_sleep_wakeup_wakeup_time").prop("value").split(":").map((s) => parseInt(s, 10))
        }
    };

    $("#configuration :input").prop("disabled", true);
    $("#message").fadeOut(250, () => $("#message").prop("class", "info").text("Sending").fadeIn(250));
    $.ajax({
        type: "POST",
        url: "http://192.168.1.200/configuration.json",
        contentType: 'application/json',
        timeout: 3000,
        data: JSON.stringify(data)
    })
        .done(() => {
            $("#configuration :input").prop("disabled", false);
            $("#message").fadeOut(250, () => $("#message").prop("class", "success").text("Done").fadeIn(250, () => $("#message").delay(2000).fadeOut(250)));
        })
        .fail((xhr, status, error) => {
            $("#configuration :input").prop("disabled", false);
            $("#message").fadeOut(250, () => $("#message").prop("class", "error").text(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`).fadeIn(250));
        });
}

function getConfiguration() {
    $("#configuration :input").prop("disabled", true);
    $("#message").fadeOut(250, () => $("#message").prop("class", "info").text("Loading").fadeIn(250));
    $.ajax({
        type: "GET",
        url: "http://192.168.1.200/configuration.json",
        accepts: 'application/json',
        timeout: 3000
    })
        .done((data) => {
            $("#access_point_enabled").prop("checked", data.access_point.enabled);
            $("#access_point_mac").prop("value", data.access_point.mac.map((n) => n.toString(16).toUpperCase().padStart(2, "0")).join("-"));
            $("#access_point_ip").prop("value", data.access_point.ip.map((n) => n.toString(10)).join("."));
            $("#access_point_netmask").prop("value", data.access_point.netmask.map((n) => n.toString(10)).join("."));
            $("#access_point_gateway").prop("value", data.access_point.gateway.map((n) => n.toString(10)).join("."));
            $("#access_point_port").prop("value", data.access_point.port);
            $("#access_point_user").prop("value", data.access_point.user);
            $("#access_point_password").prop("value", data.access_point.password);
            $("#access_point_duration").prop("value", data.access_point.duration);

            $("#station_enabled").prop("checked", data.station.enabled);
            $("#station_mac").prop("value", data.station.mac.map((n) => n.toString(16).toUpperCase().padStart(2, "0")).join("-"));
            $("#station_ip").prop("value", data.station.ip.map((n) => n.toString(10)).join("."));
            $("#station_netmask").prop("value", data.station.netmask.map((n) => n.toString(10)).join("."));
            $("#station_gateway").prop("value", data.station.gateway.map((n) => n.toString(10)).join("."));
            $("#station_port").prop("value", data.station.port);
            $("#station_user").prop("value", data.station.user);
            $("#station_password").prop("value", data.station.password);

            $("#auto_sleep_wakeup_enabled").prop("checked", data.auto_sleep_wakeup.enabled);
            $("#auto_sleep_wakeup_sleep_time").prop("value", data.auto_sleep_wakeup.sleep_time.map((n) => n.toString(10).padStart(2, "0")).join(":"));
            $("#auto_sleep_wakeup_wakeup_time").prop("value", data.auto_sleep_wakeup.wakeup_time.map((n) => n.toString(10).padStart(2, "0")).join(":"));

            $("#configuration :input").prop("disabled", false);
            $("#message").fadeOut(250, () => $("#message").prop("class", "success").text("Done").fadeIn(250, () => $("#message").delay(2000).fadeOut(250)));
        })
        .fail((xhr, status, error) => {
            $("#configuration :input").prop("disabled", false);
            $("#message").fadeOut(250, () => $("#message").prop("class", "error").text(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`).fadeIn(250));
        });
}