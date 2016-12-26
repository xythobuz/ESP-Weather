// !!DO NOT EDIT, AUTO-GENERATED FILE!!
// Use convert-static.py to recreate this.

#ifndef __STATIC_H__
#define __STATIC_H__

#define HTML_BEGIN "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>ESP-Weather</title><script src=\"https://code.jquery.com/jquery-3.1.1.min.js\" integrity=\"sha256-hVVnYaiADRTO2PzUGmuLJr8BLUSjGIZsDYGmIJLv2b8=\" crossorigin=\"anonymous\" defer></script><script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\" integrity=\"sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa\" crossorigin=\"anonymous\" defer></script><script src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.4.0/Chart.bundle.min.js\" integrity=\"sha256-RASNMNlmRtIreeznffYMDUxBXcMRjijEaaGF/gxT6vw=\" crossorigin=\"anonymous\" defer></script><script src=\"view.js\" defer></script><link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" integrity=\"sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u\" crossorigin=\"anonymous\"><link rel=\"stylesheet\" href=\"style.css\" type=\"text/css\"><link rel=\"icon\" href=\"favicon.ico\" type=\"image/x-icon\"></head><body><script type=\"text/javascript\">"
#define HTML_END "</script><div id=\"main-part\" class=\"container-fluid\"><div class=\"page-header\"><h2>ESP-Weather</h2></div></div><div class=\"footer\"><div class=\"container-fluid\"><p class=\"text-muted\"> &copy; Copyright 2016 by Christian H&ouml;gerle und Thomas Buck </p></div></div></body></html>"
#define JS_FILE "var arrSensor = Array();\ntextAvailableSensors = \"Available Sensors\";\ntextButtonNext = \"Continue\";\n$(document).ready(function() {\n$('#main-part').append(`\n<div class=\"row\" id=\"contentDiv\">\n<div class=\"col-md-5 col-lg-5\">\n<div class=\"panel panel-primary\">\n<div class=\"panel-heading\" id=\"listSensorsHeading\">\n` + textAvailableSensors + ` (0/0)\n</div>\n<div class=\"panel-body\">\n<ul class=\"list-group\" id=\"listSensors\"></ul>\n<div id=\"alertDiv\"></div>\n<button class=\"btn btn-primary\" disabled=\"\" id=\"btnSubmit\">\n` + textButtonNext + `\n</button>\n</div>\n</div>\n</div>\n</div>`);\nvar actTime = new Date();\nactTime = actTime.getHours() + \":\" + (actTime.getMinutes() < 10 ? '0':'') + actTime.getMinutes();\n$('#listSensorsHeading').empty();\n$('#listSensorsHeading').html(textAvailableSensors + \" (0/\" + clients.length + \")\");\nvar count = [0];\njQuery.each(clients, function(index, client) {\nwebSocket(client, \"2391\", count, clients.length);\n});\n$(\"#btnSubmit\").click(function(event) {\n$('#contentDiv').empty();\ngenerateView(arrSensor, actTime);\n});\n});\nfunction webSocket(wsUri, wsPort, count, clientsCount) {\nwebsocket = new WebSocket(\"ws://\" + wsUri + \":\" + wsPort + \"/\");\nwebsocket.onopen = function(evt) {};\nwebsocket.onclose = function(evt) {};\nwebsocket.onmessage = function(evt) {\nvar jsonData = jQuery.parseJSON(evt.data);\ncount[0]++;\nvar sensor = {id: count[0], ip: wsUri, actualTemp: jsonData['T'], actualHum: jsonData['H']};\nvar arrEEPROM = Array();\njQuery.each(jsonData['EEPROM'], function(index, data) {\narrEEPROM.push(data);\n});\nsensor.arrEEPROM = arrEEPROM;\narrSensor.push(sensor);\n$('#listSensorsHeading').html(textAvailableSensors + \" (\" + sensor.id + \"/\" + clientsCount + \")\");\n$('#listSensors').append('<li class=\"list-group-item\">' +\n' Sensor ' + sensor.id +\n' | IP: ' + sensor.ip +\n' | aktuelle Temperatur: ' + sensor.actualTemp +\n' | aktuelle Luftfeuchtigkeit: ' + sensor.actualHum +\n'</li>');\nif(count[0] == clientsCount) {\n$('#btnSubmit').prop(\"disabled\", false);\n}\n};\nwebsocket.onerror = function(evt) {\nif($('#websocketError').length ) {\n$('.alert-danger').append('Sensor mit der IP:' + wsUri + ' konnte nicht abgefragt werden! <br>');\n} else {\n$('#alertDiv').append('<div class=\"alert alert-danger\" id=\"websocketError\">' +\n'<strong>Fehler:</strong><br>Sensor mit der IP:' + wsUri + ' konnte nicht abgefragt werden! <br>' +\n'</div>');\n}\nconsole.log(evt.data);\n};\n}\nfunction generateView(arrSensor, actTime) {\n$('#contentDiv').append(`<div class=\"col-md-12 col-lg-12\">\n<div class=\"panel panel-primary\">\n<ul class=\"nav nav-pills\">\n<li class=\"active\"><a class=\"navtab\" data-toggle=\"tab\" href=\"#home\">Home</a></li>\n</ul>\n<div class=\"panel-body\">\n<div id=\"contentPanel\">\n</div>\n</div>\n</div>\n</div>`);\njQuery.each(arrSensor, function(index, sensor) {\n$('.nav-pills').append('<li><a class=\"navtab\" data-toggle=\"tab\" href=\"#' + sensor.id + '\">Sensor ' + sensor.id + '</a></li>');\n});\ngenerateGraph(true, arrSensor, actTime);\n$(\".navtab\").click(function(event) {\n$('#contentPanel').empty();\nif(event.target.text == \"Home\") {\ngenerateGraph(true, arrSensor, actTime);\n} else {\ngenerateGraph(false, arrSensor[(event.target.text.split(\" \")[1] - 1)], actTime);\n}\n});\n}\nfunction generateGraph(flag, sensor, actTime) {\n$('#contentPanel').append(`<div class=\"row\">\n<div class=\"col-sm-12 col-md-12 col-lg-6\">\n<canvas id=\"temperaturChart\"></canvas>\n</div>\n<div class=\"col-sm-12 col-md-12 col-lg-6\">\n<canvas id=\"humidityChart\"></canvas>\n</div>\n</div>`);\nif(flag) { var length = 0;\njQuery.each(sensor, function(index, tmp) {\nif(length < tmp.arrEEPROM.length) {\nlength = tmp.arrEEPROM.length;\n}\n});\nvar labels = Array();\nactHour = actTime.split(\":\")[0];\nfor(var i = length; i > 0; i--) {\nlabels.unshift(actHour + \":00\");\nactHour = (actHour - 1).mod(24);\n}\nlabels.push(actTime);\nvar dataTemperature = Array();\nvar dataHumidity = Array();\nvar tmpDataTemperature = Array();\nvar tmpDataHumidity = Array();\njQuery.each(sensor, function(index, tmp) {\nfor(var i = 0; i < (length - tmp.arrEEPROM.length); i++) {\ntmpDataTemperature.push([]);\ntmpDataHumidity.push([]);\n}\njQuery.each(tmp.arrEEPROM, function(index, value) {\ntmpDataTemperature.push(value['T']);\ntmpDataHumidity.push(value['H']);\n});\ntmpDataTemperature.push(tmp.actualTemp);\ntmpDataHumidity.push(tmp.actualHum);\nvar lineColor = getRandomColor();\ndataTemperature.push({label: \"Sensor \" + tmp.id, data: tmpDataTemperature, fill: false,\nborderWidth: 3, borderColor : lineColor,});\ndataHumidity.push({label: \"Sensor \" + tmp.id, data: tmpDataHumidity, fill: false,\nborderWidth: 3, borderColor : lineColor,});\ntmpDataTemperature = [];\ntmpDataHumidity = [];\n});\n} else { var labels = Array();\nvar tmpDataTemperature = Array();\nvar tmpDataHumidity = Array();\nactHour = actTime.split(\":\")[0];\nactHour = (actHour - sensor.arrEEPROM.length).mod(24);\njQuery.each(sensor.arrEEPROM, function(index, value) {\nactHour = (actHour + 1).mod(24);\nlabels.push(actHour + \":00\");\ntmpDataTemperature.push(value['T']);\ntmpDataHumidity.push(value['H']);\n});\nlabels.push(actTime);\ntmpDataTemperature.push(sensor.actualTemp);\ntmpDataHumidity.push(sensor.actualHum);\nvar dataTemperature = [{label: 'Temperatur [C]', data: tmpDataTemperature,\nfill: false, borderWidth: 3, borderColor: '#337ab7',}];\nvar dataHumidity = [{label: 'Luftfeuchtigkeit [%RH]', data: tmpDataHumidity,\nfill: false, borderWidth: 3, borderColor: '#337ab7',}];\n}\nvar tempCtx = $('#temperaturChart');\nvar humCtx = $('#humidityChart');\nvar tempChart = new Chart(tempCtx, {\ntype: 'line',\ndata: {\nlabels: labels,\ndatasets: dataTemperature,\n},\noptions: {\ntitle: {\ndisplay: true,\ntext: 'Temperaturverlauf'\n}\n}\n});\nvar humCharts = new Chart(humCtx, {\ntype: 'line',\ndata: {\nlabels: labels,\ndatasets: dataHumidity,\n},\noptions: {\ntitle: {\ndisplay: true,\ntext: 'Luftfeuchtigkeitverlauf'\n}\n}\n});\n}\nNumber.prototype.mod = function(n) {\nreturn ((this%n)+n)%n;\n}\nfunction getRandomColor() {\nvar letters = '0123456789ABCDEF'.split('');\nvar color = '#';\nfor (var i = 0; i < 6; i++ ) {\ncolor += letters[Math.floor(Math.random() * 16)];\n}\nreturn color;\n}"
#define CSS_FILE "#listSensorsHeading { font-size: 18px; }"

const static unsigned int faviconSize = 1406;
const static char faviconMimeType[] PROGMEM = "image/x-icon";
const static unsigned char favicon[] PROGMEM = {
    0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 
    0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0x68, 0x05, 
    0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x28, 0x00, 
    0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 
    0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xff, 0xe8, 0xc9, 0x00, 0xff, 0xde, 
    0xb3, 0x00, 0xff, 0x91, 0x00, 0x00, 0x3d, 0xf9, 
    0xff, 0x00, 0xff, 0xd3, 0x99, 0x00, 0x01, 0x29, 
    0x00, 0x00, 0xeb, 0x85, 0x00, 0x00, 0x02, 0x8c, 
    0x00, 0x00, 0x01, 0x40, 0x00, 0x00, 0xd1, 0x77, 
    0x00, 0x00, 0x01, 0x52, 0x00, 0x00, 0xff, 0xba, 
    0x61, 0x00, 0x02, 0x6b, 0x00, 0x00, 0xff, 0xb1, 
    0x4a, 0x00, 0xff, 0xa1, 0x26, 0x00, 0xff, 0xf3, 
    0xe3, 0x00, 0xff, 0xc4, 0x78, 0x00, 0xe3, 0xfe, 
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x0b, 
    0x0d, 0x0d, 0x0d, 0x08, 0x08, 0x08, 0x08, 0x08, 
    0x08, 0x08, 0x08, 0x0d, 0x0d, 0x0d, 0x09, 0x09, 
    0x0b, 0x0b, 0x0d, 0x0d, 0x0d, 0x0d, 0x08, 0x0d, 
    0x0d, 0x0d, 0x0d, 0x0b, 0x0b, 0x0b, 0x06, 0x06, 
    0x09, 0x09, 0x0b, 0x0b, 0x0b, 0x0d, 0x0d, 0x0d, 
    0x0b, 0x0b, 0x0b, 0x0b, 0x09, 0x09, 0x00, 0x00, 
    0x06, 0x09, 0x09, 0x09, 0x0b, 0x0b, 0x0b, 0x0b, 
    0x0b, 0x0b, 0x09, 0x09, 0x06, 0x06, 0x10, 0x10, 
    0x00, 0x06, 0x06, 0x06, 0x09, 0x09, 0x09, 0x09, 
    0x09, 0x09, 0x06, 0x06, 0x00, 0x00, 0x10, 0x10, 
    0x10, 0x00, 0x00, 0x00, 0x06, 0x09, 0x09, 0x06, 
    0x06, 0x06, 0x00, 0x00, 0x10, 0x10, 0x01, 0x01, 
    0x01, 0x01, 0x01, 0x00, 0x00, 0x06, 0x06, 0x00, 
    0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 
    0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x02, 
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x05, 0x05, 
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x11, 0x11, 
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0c, 0x0c, 
    0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 
    0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0e, 0x0e, 
    0x0e, 0x04, 0x04, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 
    0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 
    0x04, 0x12, 0x12, 0x04, 0x0f, 0x0f, 0x0f, 0x0f, 
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x03, 0x03, 
    0x04, 0x12, 0x12, 0x04, 0x03, 0x03, 0x03, 0x03, 
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x07, 0x07, 
    0x07, 0x04, 0x04, 0x07, 0x07, 0x07, 0x07, 0x07, 
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x0a, 0x0a, 
    0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 
    0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#endif // __STATIC_H__

