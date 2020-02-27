$(document).ready(() => {
    handleConfiguration();

    getConfiguration().done(() => clearMessage());
});

function handleConfiguration(){
    $("#configuration").submit((event) => {
        event.preventDefault();
        if ($("#configuration")[0].checkValidity()) {
            setConfiguration();
        }
    });
}

function setConfiguration() {
    var cfg = {
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
        },
        sensors: []
    };

    $("#sensors tbody tr").each((i,s) => {
        cfg.sensors.push({
            enabled: $(`#sensors_enabled_${i}`).prop("checked"),
            name: $(`#sensors_name_${i}`).prop("value"),
            type: parseInt($(`#sensors_type_${i}`).prop("value")),
            min: parseFloat($(`#sensors_min_${i}`).prop("value")),
            max: parseFloat($(`#sensors_max_${i}`).prop("value")),
            calibration: {
                factor: parseFloat($(`#sensors_calibration_factor_${i}`).prop("value")),
                offset: parseFloat($(`#sensors_calibration_offset_${i}`).prop("value"))
            },
            alarm: {
                enabled: $(`#sensors_alarm_enabled_${i}`).prop("checked"),
                value: parseFloat($(`#sensors_alarm_value_${i}`).prop("value"))
            }
        });
    });

    $("#configuration :input").prop("disabled", true);
    $("#message").fadeOut(250, () => $("#message").prop("class", "info").text("Sending").fadeIn(250));
    $.ajax({
        type: "POST",
        url: "http://192.168.1.200/configuration.json",
        contentType: 'application/json',
        timeout: 5000,
        data: JSON.stringify(cfg)
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
    var deferred = new $.Deferred();
    $("#configuration :input").prop("disabled", true);
    infoMessage("Loading");
    $.ajax({
        type: "GET",
        url: "http://192.168.1.200/configuration.json",
        accepts: 'application/json',
        timeout: 5000
    })
        .done((cfg) => {
            $("#access_point_enabled").prop("checked", cfg.access_point.enabled);
            $("#access_point_mac").prop("value", cfg.access_point.mac.map((n) => n.toString(16).toUpperCase().padStart(2, "0")).join("-"));
            $("#access_point_ip").prop("value", cfg.access_point.ip.map((n) => n.toString(10)).join("."));
            $("#access_point_netmask").prop("value", cfg.access_point.netmask.map((n) => n.toString(10)).join("."));
            $("#access_point_gateway").prop("value", cfg.access_point.gateway.map((n) => n.toString(10)).join("."));
            $("#access_point_port").prop("value", cfg.access_point.port);
            $("#access_point_user").prop("value", cfg.access_point.user);
            $("#access_point_password").prop("value", cfg.access_point.password);
            $("#access_point_duration").prop("value", cfg.access_point.duration);

            $("#station_enabled").prop("checked", cfg.station.enabled);
            $("#station_mac").prop("value", cfg.station.mac.map((n) => n.toString(16).toUpperCase().padStart(2, "0")).join("-"));
            $("#station_ip").prop("value", cfg.station.ip.map((n) => n.toString(10)).join("."));
            $("#station_netmask").prop("value", cfg.station.netmask.map((n) => n.toString(10)).join("."));
            $("#station_gateway").prop("value", cfg.station.gateway.map((n) => n.toString(10)).join("."));
            $("#station_port").prop("value", cfg.station.port);
            $("#station_user").prop("value", cfg.station.user);
            $("#station_password").prop("value", cfg.station.password);

            $("#auto_sleep_wakeup_enabled").prop("checked", cfg.auto_sleep_wakeup.enabled);
            $("#auto_sleep_wakeup_sleep_time").prop("value", cfg.auto_sleep_wakeup.sleep_time.map((n) => n.toString(10).padStart(2, "0")).join(":"));
            $("#auto_sleep_wakeup_wakeup_time").prop("value", cfg.auto_sleep_wakeup.wakeup_time.map((n) => n.toString(10).padStart(2, "0")).join(":"));

            var template = $($.parseHTML($("#sensor_template").html()));
            for (const [i, s] of cfg.sensors.entries()) {
                var row = template.clone();
                row.find("#sensors_number").text(i + 1);
                row.find("#sensors_enabled").prop("checked", s.enabled);
                row.find("#sensors_name").prop("value", s.name);
                row.find("#sensors_type").prop("value", s.type);
                row.find("#sensors_min").prop("value", s.min);
                row.find("#sensors_max").prop("value", s.max);
                row.find("#sensors_calibration_factor").prop("value", s.calibration.factor);
                row.find("#sensors_calibration_offset").prop("value", s.calibration.offset);
                row.find("#sensors_alarm_enabled").prop("checked", s.alarm.enabled);
                row.find("#sensors_alarm_value").prop("value", s.alarm.value);
                for (var c of row.find("*")) {
                    if (c.id) {
                        c.id += `_${i}`;
                    }
                    if (c.htmlFor) {
                        c.htmlFor += `_${i}`;
                    }
                }
                row.appendTo($("#sensors tbody"));
            }
            successMessage("Done");
            deferred.resolve();
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            $("#configuration :input").prop("disabled", false);
        });
        return deferred.promise();
}

function clearMessage() {
    if (typeof this.fadeOutHandle != "undefined") {
        clearTimeout(this.fadeOutHandle);
    }
    this.fadeOutHandle = setTimeout(() => $("#message").fadeOut(250), 2000);
}

function infoMessage(text) {
    return $("#message").prop("class", "info").text(text).fadeTo(250, 1.0).promise();
}

function successMessage(text) {
    return $("#message").prop("class", "success").text(text).fadeTo(250, 1.0).promise();
}

function warningMessage(text) {
    return $("#message").prop("class", "warning").text(text).fadeTo(250, 1.0).promise();
}

function errorMessage(text) {
    return $("#message").prop("class", "error").text(text).fadeTo(250, 1.0).promise();
}