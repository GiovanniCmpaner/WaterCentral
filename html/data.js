$(document).ready(() => {
    handleFilter();
    getDateTime().done(() => handleLoadMore());
});

function handleFilter() {
    $("#filter").submit((event) => {
        event.preventDefault();
        if ($("#filter")[0].checkValidity()) {
            handleLoadMore();
        }
    });
}

function handleDownload() {
    
}

function handleLoadMore(withoutFilter) {
    $(window).unbind("scroll");
    clearTimeout(this.updateHandle);

    getData(withoutFilter ? null : buildfilter())
        .done((count) => {
            if (count == 20) {
                $(window).scroll(() => {
                    if ($(window).height() + $(window).scrollTop() > $("body").height() * 0.75) {
                        $(window).unbind("scroll");
                        handleLoadMore(true);
                    }
                });
            }
            else {
                this.updateHandle = setTimeout(() => handleLoadMore(true), 30000);
            }
        })
        .then(clearMessage);
}

function getDateTime() {
    var deferred = new $.Deferred();

    $("#filter :input").prop("disabled", true);
    infoMessage("Loading").then(() => {
        $.ajax({
            type: "GET",
            url: "http://192.168.1.200/datetime.json",
            accepts: 'application/json',
            timeout: 5000
        })
            .done((data) => {
                var dateTime = data.datetime.split(" ");
                $("#filter_start_date").val(dateTime[0]);
                $("#filter_start_time").val("00:00:00");
                $("#filter_end_date").val(dateTime[0]);
                $("#filter_end_time").val("23:59:59");

                successMessage("Done");
                deferred.resolve(new Date(data.datetime));
            })
            .fail((xhr, status, error) => {
                errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
                deferred.reject();
            })
            .always(() => {
                $("#filter :input").prop("disabled", false);
            });
    });
    return deferred.promise();
}

function buildfilter() {
    var filter = {};
    if ($("#filter_id").val()) {
        filter.id = $("#filter_id").val();
    }
    else {
        delete filter.id;
    }
    if ($("#filter_start_date").val() && $("#filter_start_time").val()) {
        filter.start = `${$("#filter_start_date").val()} ${$("#filter_start_time").val()}`;
    }
    else {
        delete filter.start;
    }
    if ($("#filter_end_date").val() && $("#filter_end_time").val()) {
        filter.end = `${$("#filter_end_date").val()} ${$("#filter_end_time").val()}`;
    }
    else {
        delete filter.end;
    }
    return filter;
}

function getData(filter) {
    var deferred = new $.Deferred();

    var clear = true;
    if (filter == null) {
        clear = false;
        if (typeof this.prevFilter != "undefined") {
            filter = this.prevFilter;
        }
    }

    $("#filter :input").prop("disabled", true);
    infoMessage("Loading");
    $.ajax({
        type: "GET",
        url: "http://192.168.1.200/data.json",
        accepts: 'application/json',
        timeout: 5000,
        data: filter
    })
        .done((data) => {
            if (clear) {
                $("#result tbody tr").remove();
            }

            let template = $($.parseHTML($("#data_template").html()));
            for (const [i, d] of data.entries()) {
                let row = template.clone();
                let [date, time] = d.datetime.split(" ");
                row.find("#data_id").text(d.id);
                row.find("#data_date").text(date);
                row.find("#data_time").text(time);
                row.find("#data_temperature").text(d.temperature);
                row.find("#data_humidity").text(d.humidity);
                row.find("#data_pressure").text(d.pressure);
                row.find("#data_sensor_0").text(d.sensors[0]);
                row.find("#data_sensor_1").text(d.sensors[1]);
                row.find("#data_sensor_2").text(d.sensors[2]);
                for (let c of row.find("*")) {
                    if (c.id) {
                        c.id += `_${i}`;
                    }
                    if (c.htmlFor) {
                        c.htmlFor += `_${i}`;
                    }
                }
                row.appendTo($("#result tbody"));
            }

            this.prevFilter = filter;
            if (data.length > 0) {
                if (this.prevFilter != null) {
                    this.prevFilter.id = data[data.length - 1].id + 1;
                }
                else {
                    this.prevFilter = { id: data[data.length - 1].id + 1 };
                }
            }

            successMessage("Done");
            deferred.resolve(data.length);
        })
        .fail((xhr, status, error) => {
            errorMessage(status == "timeout" ? "Fail: Timeout" : `Fail: ${xhr.status} ${xhr.statusText}`);
            deferred.reject();
        })
        .always(() => {
            $("#filter :input").prop("disabled", false);
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