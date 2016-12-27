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

// Text Strings. Change these to translate.
textAvailableSensors = "Available Sensors";
textButtonNext = "Continue";
backTabName = '<span class="glyphicon glyphicon-step-backward" aria-hidden="true"></span>';
homeTabName = '<span class="glyphicon glyphicon-home" aria-hidden="true"></span>';
sensorTabName = "Sensor";
errorMessage = "Couldn't read sensor with IP: ";
errorTitle = "Error: ";
temperatureLabel = 'Temperature [C]';
humidityLabel = 'Humidity [%RH]';
temperatureHeading = "Temperature";
humidityHeading = "Humidity";

// Colors
singleChartTempColor = "#337ab7";
singleChartHumidColor = "#337ab7";
preDefinedColors = Array(
    "#337ab7", "#ff0000", "#00ff00"
);

// Get current client-time for the graph X-axis labels
var actTime = new Date();
actTime = actTime.getHours() + ":" + (actTime.getMinutes() < 10 ? '0':'') + actTime.getMinutes();

var arrSensor = Array(); // Data received from Websockets
var currentState = "initial"; // "initial", "main", "1", "2", ...

// Draw initial view when the page has been loaded
$(document).ready(resizeAndRedraw);

// Match graph canvases to their parent containers on resize
$(window).on('resize', resizeAndRedraw, false);

// Redraw current graph view, used for resizing
function redraw(animate) {
    if (currentState === "initial") {
        initialView();
    } else if (currentState === "main") {
        generateView(arrSensor, animate);
    } else {
        var n = Math.floor(Number(currentState));
        if ((String(n) === currentState) && (n >= 0)) {
            generateGraph(false, arrSensor[(currentState - 1)], animate);
        } else {
            console.log("Invalid state: " + currentState);
        }
    }
}

function resize() {
    $("canvas").each(function(i, el) {
        $(el).attr({
            "width":$(el).parent().width(),
            "height":$(el).parent().outerHeight()
        });
    });
}

function resizeAndRedraw(animate) {
    resize();
    redraw(animate);
}

function initialView() {
    // Show first page and hide the graphs
    $('#startDiv').show();
    $('#dataDiv').hide();

    // Reset the button text
    $('#btnSubmit').empty();
    $('#btnSubmit').html(textButtonNext);

    // Reset heading with number of clients
    $('#listSensorsHeading').empty();
    $('#listSensorsHeading').html(textAvailableSensors + " (0/" + clients.length + ")");

    // Iterate all given client IPs and get their data using Websocket
    $('#listSensors').empty();
    arrSensor = Array();
    var count = [0];
    jQuery.each(clients, function(index, client) {
        webSocket(client, "2391", count, clients.length);
    });

    // Button to continue to graph view
    $("#btnSubmit").click(function(event) {
        currentState = "main";
        resizeAndRedraw(true);
    });
}

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
        console.log("WebSocket Error: " + evt.data);
    };
}

function generateView(arrSensor, animation) {
    // Hide first page, show graph page
    $('#startDiv').hide();
    $('#dataDiv').show();

    // Add home tab
    $('#navbar').empty();
    $('#navbar').append('<li><a id="backbut" class="navtab" data-toggle="tab" href="#start" aria-label="start page">' + backTabName + '</a></li>');
    $('#navbar').append('<li class="active"><a id="homebut" class="navtab" data-toggle="tab" href="#home">' + homeTabName + '</a></li>');

    if (arrSensor.length > 1) {
        // Add tabs for all sensors
        jQuery.each(arrSensor, function(index, sensor) {
            $('#navbar').append('<li><a id="sensbut" class="navtab" data-toggle="tab" href="#' + sensor.id + '">' + sensorTabName + ' ' + sensor.id + '</a></li>');
        });
    }

    // flag for combined plot -> true
    generateGraph(true, arrSensor, animation);

    // Handler for "back" button, returning to first page
    $("#backbut").click(function(event) {
        currentState = "initial";
        redraw();
    });

    // Handler for "Home" button, drawing combined graph
    $("#homebut").click(function(event) {
        // flag for combined plot -> true
        generateGraph(true, arrSensor, true);
        currentState = "main";
    });

    if (arrSensor.length > 1) {
        // Handler for single sensor buttons
        $("#sensbut").click(function(event) {
            generateGraph(false, arrSensor[(event.target.text.split(" ")[1] - 1)], true);
            currentState = event.target.text.split(" ")[1];
        });
    }
}

function generateGraph(flag, sensor, anim) {
    resize();

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

            var lineColor = getColor(index);
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
                                fill: false, borderWidth: 3, borderColor: singleChartTempColor,}];
        var dataHumidity = [{label: humidityLabel, data: tmpDataHumidity,
                                fill: false, borderWidth: 3, borderColor: singleChartHumidColor,}];
    }

    var tempCtx = $('#temperatureChart');
    var humCtx = $('#humidityChart');

    var tempOptions = {
        title: {
            display: true,
            text: temperatureHeading
        },
        responsive: true,
        maintainAspectRatio: true,
        scaleOverride: true,
    };

    var humOptions = {
        title: {
            display: true,
            text: humidityHeading
        },
        responsive: true,
        maintainAspectRatio: true,
        scaleOverride: true,
    };

    if (!anim) {
        tempOptions.animation = false;
        humOptions.animation = false;
    }

    var tempChart = new Chart(tempCtx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: dataTemperature,
        },
        options: tempOptions
    });

    var humCharts = new Chart(humCtx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: dataHumidity,
        },
        options: humOptions
    });
}

// Modulo-Bug: http://javascript.about.com/od/problemsolving/a/modulobug.htm
Number.prototype.mod = function(n) {
    return ((this%n)+n)%n;
}

function getColor(index) {
    if (index < preDefinedColors.length) {
        return preDefinedColors[index];
    } else {
        return getRandomColor();
    }
}

function getRandomColor() {
    var letters = '0123456789ABCDEF'.split('');
    var color = '#';
    for (var i = 0; i < 6; i++ ) {
        color += letters[Math.floor(Math.random() * 16)];
    }
    return color;
}

