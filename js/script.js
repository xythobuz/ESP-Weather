var arrSensor = Array();

$(document).ready(function() {

    $('body').append(`<div class="container-fluid">
            <div class="page-header">
                <h2>Systemadministration</h2>
            </div>
            <div class="row" id="contentDiv">
                <div class="col-md-5 col-lg-5">
                    <div class="panel panel-primary">
                        <div class="panel-heading" id="listSensorsHeading" style="font-size: 18px;">Verf&uuml;gbare Sensoren (0/0)</div>
                        <div class="panel-body">
                            <ul class="list-group" id="listSensors"></ul>
                            <div id="alertDiv"></div>
                            <button class="btn btn-primary" disabled="" id="btnSubmit">Weiter</button>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        <div class="footer">
            <div class="container-fluid">
                <p class="text-muted">&copy; Copyright 2016 by Christian H&ouml;gerle und Thomas Buck</p>
            </div>
        </div>`);

    // aktuelle Uhrzeit fuer die Graphen ermitteln
    var actTime = new Date();
    actTime = actTime.getHours() + ":" + (actTime.getMinutes() < 10 ? '0':'') + actTime.getMinutes(); 

    $('#listSensorsHeading').empty();
    $('#listSensorsHeading').html("Verf&uuml;gbare Sensoren (0/" + clients.length + ")");
    
    // Alle clients im Array iterieren und Websocket abfragen
    var count = [0];
    jQuery.each(clients, function(index, client) {
        webSocket(client, "2391", count, clients.length);
    });

    // Submit-Button fuer die Formulareingabe
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
        var sensor = {id: count[0], ip: wsUri, actualTemp: jsonData['T'], actualHum: jsonData['H']};
        var arrEEPROM = Array();
        jQuery.each(jsonData['EEPROM'], function(index, data) {
            arrEEPROM.push(data);
        });
        sensor.arrEEPROM = arrEEPROM;
        arrSensor.push(sensor);
        $('#listSensorsHeading').html("Verf&uuml;gbare Sensoren (" + sensor.id + "/" + clientsCount + ")");
        $('#listSensors').append('<li class="list-group-item">' +
                                    ' Sensor ' + sensor.id +  
                                    ' | IP: ' + sensor.ip + 
                                    ' | aktuelle Temperatur: ' + sensor.actualTemp + 
                                    ' | aktuelle Luftfeuchtigkeit: ' + sensor.actualHum + 
                                '</li>');

        // Alle Sensoren erfolgreich abgefragt
        if(count[0] == clientsCount) {
            $('#btnSubmit').prop("disabled", false);
        }
    };
    websocket.onerror = function(evt) {
        if($('#websocketError').length ) {
            $('.alert-danger').append('Sensor mit der IP:' + wsUri + ' konnte nicht abgefragt werden! <br>');
        } else {
            $('#alertDiv').append('<div class="alert alert-danger" id="websocketError">' +
                                    '<strong>Fehler:</strong><br>Sensor mit der IP:' + wsUri + ' konnte nicht abgefragt werden! <br>' +
                              '</div>');
        }
        console.log(evt.data);
    };
} 

function generateView(arrSensor, actTime) {
    $('#contentDiv').append(`<div class="col-md-12 col-lg-12">
                                <div class="panel panel-primary">
                                    <ul class="nav nav-pills">
                                        <li class="active"><a class="navtab" data-toggle="tab" href="#home">Home</a></li>
                                    </ul>
                                    <div class="panel-body">
                                        <div id="contentPanel">
                                        </div>
                                    </div>
                                </div>
                            </div>`);

    jQuery.each(arrSensor, function(index, sensor) {
        $('.nav-pills').append('<li><a class="navtab" data-toggle="tab" href="#' + sensor.id + '">Sensor ' + sensor.id + '</a></li>');
    });
    
    // Flag fuer gemeinsamer Graph -> true
    generateGraph(true, arrSensor, actTime);
    
    $(".navtab").click(function(event) {
        $('#contentPanel').empty();
        if(event.target.text == "Home") {
            // Flag fuer gemeinsamer Graph -> true
            generateGraph(true, arrSensor, actTime);
        } else {
            generateGraph(false, arrSensor[(event.target.text.split(" ")[1] - 1)], actTime);
        }
    });
}

function generateGraph(flag, sensor, actTime) {
    $('#contentPanel').append(`<div class="row">
                                <div class="col-md-6 col-lg-6">
                                    <canvas id="temperaturChart"></canvas>
                                </div>
                                <div class="col-md-6 col-lg-6">
                                    <canvas id="humidityChart"></canvas>
                                </div>
                            </div>`);
    if(flag) { // ein Graph für alle Sensoren
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
            for(var i = 0; i < (length - tmp.arrEEPROM.length); i++) {
                tmpDataTemperature.push([]);
                tmpDataHumidity.push([]);
            }
            jQuery.each(tmp.arrEEPROM, function(index, value) { 
                tmpDataTemperature.push(value['T']);
                tmpDataHumidity.push(value['H']);  
            });
            tmpDataTemperature.push(tmp.actualTemp);
            tmpDataHumidity.push(tmp.actualHum);
            
            dataTemperature.push({label: "Sensor " + tmp.id, data: tmpDataTemperature, fill: false,
                borderWidth: 3, borderColor : getRandomColor(),});
            dataHumidity.push({label: "Sensor " + tmp.id, data: tmpDataHumidity, fill: false,
                borderWidth: 3, borderColor : getRandomColor(),});

            tmpDataTemperature = [];
            tmpDataHumidity = [];
        });
    } else { // Graph für einzelnen Sensor
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
        tmpDataTemperature.push(sensor.actualTemp);
        tmpDataHumidity.push(sensor.actualHum);

        var dataTemperature = [{label: 'Temperatur [C]', data: tmpDataTemperature, 
                                fill: false, borderWidth: 3, borderColor: '#337ab7',}];
        var dataHumidity = [{label: 'Luftfeuchtigkeit [%RH]', data: tmpDataHumidity, 
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
                text: 'Temperaturverlauf'
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
                text: 'Luftfeuchtigkeitverlauf'
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