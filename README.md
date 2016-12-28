# ESP-Weather

This [started out as a project](https://github.com/g40st/Systemadministration) for the "Systemadministration" course in the Hochschule Ravensburg-Weingarten in the winter of 2016 / 2017.

## Idea

The core concept of this project is to create very cheap WiFi temperature and humidity sensors that can be placed in different locations inside the same wireless network. When any one is opened by the user using a Webbrowser, it will automatically discover all other available modules in the local network (using a UDP broadcast) and visualize their data values. The procedure is visualized in the following animation:

[![Connection order animation](https://i.imgur.com/6t5W36G.gif)](https://i.imgur.com/6t5W36G.gif)

## Hardware

You need an Espressif ESP8266 module. We've used the smallest and cheapest available one, [ESP-01](http://www.esp8266.com/wiki/doku.php?id=esp8266-module-family#esp-01). The temperature and humidity sensor in use is the [Sensirion SHT21](https://www.sensirion.com/de/produkte/feuchtesensoren/feuchte-temperatursensor-sht2x-digital-i2c-genauigkeit/) because of it's low price, digital interface, good accuracy and many cheap generic breakout boards available.

[![Hardware Schematic](https://i.imgur.com/1maI2lO.png)](https://i.imgur.com/1maI2lO.png)

## Build Steps

To avoid having to do multiple steps on firmware changes, all static data is compiled into the firmware itself. The included Python 3 script `convert-static.py` should be used to convert the static files (HTML, CSS, JS and the Favicon) into the `static.h` header used by the code.

When doing changes to any static files, simpy run `./convert-static.py` and then build and upload in the Arduino IDE.

## Local Webinterface development

To quickly test changes on the frontend code without having to recompile, use the `test.html` file in a Webbrowser. You need to have an ESP-Weather module running in your network and enter the IP in the html page.

Of course, any changes to `template.html` should also be done to `test.html`.

## Dependencies

The included Favicon is ['weather' by 'jkeks'](http://www.favicon.cc/?action=icon&file_id=757061).

### ESP8266

Download and install the following libraries to your `Arduino/libraries` folder. Of course, you also need the [Arduino core for the ESP8266 WiFi chip](https://github.com/esp8266/Arduino) installed from the IDE Board Manager.

* [Temperature and Humidity Sensor SHT21](https://github.com/markbeee/SHT21)
* [Websocket Server](https://github.com/morrissinger/ESP8266-Websocket)
* [WiFiManager](https://github.com/tzapu/WiFiManager)

### JavaScript

The JavaScript client code includes the following libraries, using their official CDNs:

* [Bootstrap](http://getbootstrap.com/)
* [jQuery](https://jquery.com/)
* [Chart.js](https://github.com/chartjs/Chart.js)

## License

    ----------------------------------------------------------------------------
    "THE BEER-WARE LICENSE" (Revision 42):
    <xythobuz@xythobuz.de> & <ghost-ghost@web.de> wrote this file.  As long as
    you retain this notice you can do whatever you want with this stuff. If we
    meet some day, and you think this stuff is worth it, you can buy us a beer
    in return.                                   Thomas Buck & Christian Högerle
    ----------------------------------------------------------------------------

