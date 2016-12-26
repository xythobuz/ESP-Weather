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
#define DEBUG_BAUDRATE 115200

// Size of EEPROM for history storage.
// 4096 should be ok, but too large values cause system crashes!
#define EEPROM_SIZE 512

// Broadcast Identifier. Change this when the protocol changes!
#define UDP_PING_CONTENTS "pingESP8266v0.1"
#define UDP_ECHO_CONTENTS "echoESP8266v0.1"

#endif // __CONFIG_H__

