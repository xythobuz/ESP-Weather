# ESP-Weather

This [started out as a project](https://github.com/g40st/Systemadministration) for the "Systemadministration" course in the Hochschule Ravensburg-Weingarten in the winter of 2016 / 2017.

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

