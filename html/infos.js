$(document).ready(() => {

    getInfos().done(drawGraphs).done(clearMessage);
    //for(var c of $("canvas"))
    //{
    //    var ctx = c.getContext("2d");
    //    ctx.canvas.width = c.offsetWidth;
    //    ctx.canvas.height = c.offsetHeight;
    //    ctx.fillStyle = "red";
    //    ctx.fillRect(0,0,c.offsetWidth,c.offsetHeight);
    //}
});

function drawGraphs(info) {
    for (const [i, sensor] of info.sensors.entries()) {
        var canvas = document.getElementById(`sensor_graph_${i}`);
        var ctx = canvas.getContext("2d");
        ctx.canvas.width = canvas.offsetWidth;
        ctx.canvas.height = canvas.offsetHeight;
        ctx.fillStyle = "blue";
        ctx.fillRect(0, 0, canvas.offsetWidth * (Math.abs(sensor.percent) / 100), canvas.offsetHeight);
    };
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

            $("#temperature").text(info.temperature);
            $("#humidity").text(info.humidity);
            $("#pressure").text(info.pressure);

            var template = $($.parseHTML($("#sensor_template").html()));
            for (const [i, sensor] of info.sensors.entries()) {
                var row = template.clone();
                row.find("#sensor_name").text(sensor.name);
                row.find("#sensor_value").text(sensor.value);
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