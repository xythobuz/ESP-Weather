/*
 * config.h
 *
 * Configuration options for ESP-Weather.
 *
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <xythobuz@xythobuz.de> & <ghost-ghost@web.de> wrote this file.  As long as
 * you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer
 * in return.                                   Thomas Buck & Christian Högerle
 * ----------------------------------------------------------------------------
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define WEB_PORT 80
#define BROADCAST_PORT 2390
#define WEBSOCKET_PORT 2391
#define NTP_PORT_FROM 2392
#define NTP_PORT_TO 123

// WiFi Credentials for the captive config portal AP
#define DEFAULT_SSID "ESP-Weather"
#define DEFAULT_PASS "testtest"

#define MAX_BROADCAST_WAIT_TIME 550
#define NTP_RETRY_TIMEOUT 2500
#define NTP_SERVER_NAME "time.nist.gov"
#define NTP_PACKET_SIZE 48
#define UDP_PACKET_BUFFER_SIZE 25

// Size of EEPROM for history storage.
// 4096 should be ok, but too large values cause system crashes!
#define EEPROM_SIZE 512

// Broadcast Identifier. Change this when the protocol changes!
#define UDP_PING_CONTENTS "pingESP8266v0.1"
#define UDP_ECHO_CONTENTS "echoESP8266v0.1"

// Using the RawGit.com service to serve the scripts directly from GitHub.
// Consider using cdn.rawgit.com to reduce their server load.
#define HTML_BEGIN "<html><head><title>ESP-Weather</title>\
<script src=\"https://code.jquery.com/jquery-3.1.1.min.js\" integrity=\"sha256-hVVnYaiADRTO2PzUGmuLJr8BLUSjGIZsDYGmIJLv2b8=\" crossorigin=\"anonymous\"></script>\
<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js\" integrity=\"sha384-Tc5IQib027qvyjSMfHjOMaLkfuWVxZxUPnCJA7l2mCWNIpG9mGCD8wGNIcPD7Txa\" crossorigin=\"anonymous\"></script>\
<script src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.4.0/Chart.bundle.min.js\" integrity=\"sha256-RASNMNlmRtIreeznffYMDUxBXcMRjijEaaGF/gxT6vw=\" crossorigin=\"anonymous\"></script>\
<script src=\"https://rawgit.com/xythobuz/ESP-Weather/master/client-script.js\"></script>\
<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css\" integrity=\"sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u\" crossorigin=\"anonymous\">\
</head><body><script type=\"text/javascript\">"

#define HTML_END "</script></body></html>"

#endif // __CONFIG_H__

