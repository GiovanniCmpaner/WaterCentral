$(document).ready(() => {

    getInfos().then((info) => drawGraphs(info)).then(() => clearMessage());
});

function drawGraphs(info) {
    //for (const [i, sensor] of info.sensors.entries()) {
    //    var canvas = document.getElementById(`sensor_graph_${i}`);
    //    var ctx = canvas.getContext("2d");
    //    //ctx.canvas.width = canvas.offsetWidth;
    //    //ctx.canvas.height = canvas.offsetHeight;
    //    ctx.fillStyle = "blue";
    //    ctx.fillRect(0, 0, canvas.offsetWidth * (Math.min(Math.max(sensor.percent, 0.0), 100.0) / 100.0), canvas.offsetHeight);
    //};
}

function getInfos() {
    var deferred = new $.Deferred();

    infoMessage("Loading");
    $.ajax({
        type: "GET",
        url: "http://192.168.1.200/infos.json",
        accepts: 'application/json',
        timeout: 5000
    })
        .done((info) => {

            if (!info.temperature || !info.humidity || !info.pressure) {
                $("#temperature").prop("class", "error").text("ERROR");
                $("#humidity").prop("class", "error").text("ERROR");
                $("#pressure").prop("class", "error").text("ERROR");
            }
            else {
                $("#temperature").removeProp("class").text(info.temperature);
                $("#humidity").removeProp("class").text(info.humidity);
                $("#pressure").removeProp("class").text(info.pressure);
            }

            var template = $($.parseHTML($("#sensor_template").html()));
            for (const [i, sensor] of info.sensors.entries()) {
                var row = template.clone();
                for (var c of row) {
                    if (c.id == "sensor_name") {
                        c.textContent = sensor.name;
                    }
                    else if (c.id == "sensor_value") {
                        c.textContent = sensor.value;
                    }
                    if (c.id) {
                        c.id += `_${i}`;
                    }
                    if (c.htmlFor) {
                        c.htmlFor += `_${i}`;
                    }
                }
                row.appendTo($("#values.grid"));
            }

            successMessage("Done");
            deferred.resolve(info);
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
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