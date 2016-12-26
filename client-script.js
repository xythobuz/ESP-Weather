// This is the client-side Javascript that receives the IPs of all available
// local ESP-Weather modules, reads their values using the Websocket interface
// and renders the graphs to visualize them.
//
// ----------------------------------------------------------------------------
// "THE BEER-WARE LICENSE" (Revision 42):
// <xythobuz@xythobuz.de> & <ghost-ghost@web.de> wrote this file.  As long as
// you retain this notice you can do whatever you want with this stuff. If we
// meet some day, and you think this stuff is worth it, you can buy us a beer
// in return.                                   Thomas Buck & Christian Högerle
// ----------------------------------------------------------------------------

var arrSensor = Array();

textAvailableSensors = "Available Sensors";
textButtonNext = "Continue";
homeTabName = "Home";
sensorTabName = "Sensor";
errorMessage = "Couldn't read sensor with IP: ";
errorTitle = "Error: ";
temperatureLabel = 'Temperature [C]';
humidityLabel = 'Humidity [%RH]';
temperatureHeading = "Temperature";
humidityHeading = "Humidity";

$(document).ready(function() {
    $('#main-part').append(`
        <div class="row" id="contentDiv">
            <div class="col-md-5 col-lg-5">
                <div class="panel panel-primary">
                    <div class="panel-heading" id="listSensorsHeading">
                        ` + textAvailableSensors + ` (0/0)
                    </div>
                    <div class="panel-body">
                        <ul class="list-group" id="listSensors"></ul>
                        <div id="alertDiv"></div>
                        <button class="btn btn-primary" disabled="" id="btnSubmit">
                            ` + textButtonNext + `
                        </button>
                    </div>
                </div>
            </div>
        </div>`);

    // Get current client-time for the graph X-axis labels
    var actTime = new Date();
    actTime = actTime.getHours() + ":" + (actTime.getMinutes() < 10 ? '0':'') + actTime.getMinutes();

    $('#listSensorsHeading').empty();
    $('#listSensorsHeading').html(textAvailableSensors + " (0/" + clients.length + ")");

    // Iterate all given client IPs and get their data using Websocket
    var count = [0];
    jQuery.each(clients, function(index, client) {
        webSocket(client, "2391", count, clients.length);
    });

    // Button to continue to graph view
    $("#btnSubmit").click(function(event) {
        $('#contentDiv').empty();
        generateView(arrSensor, actTime);
    });
});

function webSocket(wsUri, wsPort, count, clientsCount) {
    websocket = new WebSocket("ws://" + wsUri + ":" + wsPort + "/");
    websocket.onopen = function(evt) {};
    websocket.onclose = function(evt) {};
    websocket.onmessage = function(evt) {
        var jsonData = jQuery.parseJSON(evt.data);
        count[0]++;
        var sensor = {id: count[0], ip: wsUri, currentTemp: jsonData['T'], currentHum: jsonData['H']};
        var arrEEPROM = Array();
        jQuery.each(jsonData['EEPROM'], function(index, data) {
            arrEEPROM.push(data);
        });
        sensor.arrEEPROM = arrEEPROM;
        arrSensor.push(sensor);
        $('#listSensorsHeading').html(textAvailableSensors + " (" + sensor.id + "/" + clientsCount + ")");
        $('#listSensors').append('<li class="list-group-item">' +
                                    ' Sensor ' + sensor.id +
                                    ' | IP: ' + sensor.ip +
                                    ' | Temperature: ' + sensor.currentTemp +
                                    ' | Humidity: ' + sensor.currentHum +
                                '</li>');

        // Enable continue buttons when all modules have been reached
        if(count[0] == clientsCount) {
            $('#btnSubmit').prop("disabled", false);
        }
    };
    websocket.onerror = function(evt) {
        if($('#websocketError').length ) {
            $('.alert-danger').append(errorMessage + wsUri + '<br>');
        } else {
            $('#alertDiv').append('<div class="alert alert-danger" id="websocketError">' +
                                    '<strong>' + errorTitle
                                    + '</strong><br>' + errorMessage
                                    + wsUri + '<br></div>');
        }
        console.log(evt.data);
    };
}

function generateView(arrSensor, actTime) {
    $('#contentDiv').append(`<div class="col-md-12 col-lg-12">
                                <div class="panel panel-primary">
                                    <ul class="nav nav-pills">
                                        <li class="active"><a class="navtab" data-toggle="tab" href="#home">` + homeTabName + `</a></li>
                                    </ul>
                                    <div class="panel-body">
                                        <div id="contentPanel">
                                        </div>
                                    </div>
                                </div>
                            </div>`);

    jQuery.each(arrSensor, function(index, sensor) {
        $('.nav-pills').append('<li><a class="navtab" data-toggle="tab" href="#' + sensor.id + '">' + sensorTabName + ' ' + sensor.id + '</a></li>');
    });

    // flag for combined plot -> true
    generateGraph(true, arrSensor, actTime);

    $(".navtab").click(function(event) {
        $('#contentPanel').empty();
        if(event.target.text == homeTabName) {
            // flag for combined plot -> true
            generateGraph(true, arrSensor, actTime);
        } else {
            generateGraph(false, arrSensor[(event.target.text.split(" ")[1] - 1)], actTime);
        }
    });
}

function generateGraph(flag, sensor, actTime) {
    $('#contentPanel').append(`<div class="row">
                                <div class="col-sm-12 col-md-12 col-lg-6">
                                    <canvas id="temperaturChart"></canvas>
                                </div>
                                <div class="col-sm-12 col-md-12 col-lg-6">
                                    <canvas id="humidityChart"></canvas>
                                </div>
                            </div>`);
    if (flag) {
        // one plot for all sensors
        var length = 0;
        jQuery.each(sensor, function(index, tmp) {
            if(length < tmp.arrEEPROM.length) {
                length = tmp.arrEEPROM.length;
            }
        });

        var labels = Array();
        actHour = actTime.split(":")[0];
        for(var i = length; i > 0; i--) {
            labels.unshift(actHour + ":00");
            actHour = (actHour - 1).mod(24);
        }
        labels.push(actTime);

        var dataTemperature = Array();
        var dataHumidity = Array();

        var tmpDataTemperature = Array();
        var tmpDataHumidity = Array();
        jQuery.each(sensor, function(index, tmp) {
            for (var i = 0; i < (length - tmp.arrEEPROM.length); i++) {
                tmpDataTemperature.push([]);
                tmpDataHumidity.push([]);
            }
            jQuery.each(tmp.arrEEPROM, function(index, value) {
                tmpDataTemperature.push(value['T']);
                tmpDataHumidity.push(value['H']);
            });
            tmpDataTemperature.push(tmp.currentTemp);
            tmpDataHumidity.push(tmp.currentHum);

            var lineColor = getRandomColor();
            dataTemperature.push({label: sensorTabName + " " + tmp.id, data: tmpDataTemperature, fill: false,
                borderWidth: 3, borderColor : lineColor,});
            dataHumidity.push({label: sensorTabName + " " + tmp.id, data: tmpDataHumidity, fill: false,
                borderWidth: 3, borderColor : lineColor,});

            tmpDataTemperature = [];
            tmpDataHumidity = [];
        });
    } else {
        // plot for one sensor
        var labels = Array();

        var tmpDataTemperature = Array();
        var tmpDataHumidity = Array();
        actHour = actTime.split(":")[0];
        actHour = (actHour - sensor.arrEEPROM.length).mod(24);
        jQuery.each(sensor.arrEEPROM, function(index, value) {
            actHour = (actHour + 1).mod(24);
            labels.push(actHour + ":00");
            tmpDataTemperature.push(value['T']);
            tmpDataHumidity.push(value['H']);
        });

        labels.push(actTime);
        tmpDataTemperature.push(sensor.currentTemp);
        tmpDataHumidity.push(sensor.currentHum);

        var dataTemperature = [{label: temperatureLabel, data: tmpDataTemperature,
                                fill: false, borderWidth: 3, borderColor: '#337ab7',}];
        var dataHumidity = [{label: humidityLabel, data: tmpDataHumidity,
                                fill: false, borderWidth: 3, borderColor: '#337ab7',}];
    }

    var tempCtx = $('#temperaturChart');
    var humCtx = $('#humidityChart');

    var tempChart = new Chart(tempCtx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: dataTemperature,
        },
        options: {
            title: {
                display: true,
                text: temperatureHeading
            }
        }
    });

    var humCharts = new Chart(humCtx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: dataHumidity,
        },
        options: {
            title: {
                display: true,
                text: humidityHeading
            }
        }
    });
}

// Modulo-Bug: http://javascript.about.com/od/problemsolving/a/modulobug.htm
Number.prototype.mod = function(n) {
    return ((this%n)+n)%n;
}

function getRandomColor() {
    var letters = '0123456789ABCDEF'.split('');
    var color = '#';
    for (var i = 0; i < 6; i++ ) {
        color += letters[Math.floor(Math.random() * 16)];
    }
    return color;
}

