/*
 * ESP-Weather.ino
 *
 * This is the main program for our distributed temperature and humidity logger
 * with WiFi interface, based on the ESP8266 ESP-01 module and an SHT21 sensor.
 *
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <xythobuz@xythobuz.de> & <ghost-ghost@web.de> wrote this file.  As long as
 * you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer
 * in return.                                   Thomas Buck & Christian Högerle
 * ----------------------------------------------------------------------------
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <SHT21.h>
#include <vector>
#include <WebSocketServer.h>
#include <WiFiServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include "config.h"
#include "ntp.h"
#include "static.h"
#include "storage.h"

#define DEBUG

SHT21 sensor;
ESP8266WebServer server(WEB_PORT);
WiFiServer serverSocket(WEBSOCKET_PORT);
WebSocketServer webSocketServer;
IPAddress broadcastIP;
WiFiUDP udp;
PersistentStorage storage;
std::vector<IPAddress> vecClients; 
char packetBuffer[UDP_PACKET_BUFFER_SIZE];
unsigned long lastStorageTime = 0;
byte storeAtBoot = 1;
unsigned long lastTime = 0;
bool waitingForReplies = false;

static void handleRoot() {
#ifdef DEBUG
    Serial.println("Sending UDP Broadcast...");
#endif // DEBUG

    // Send UDP broadcast to other modules
    udp.beginPacket(broadcastIP, BROADCAST_PORT);
    udp.write(UDP_PING_CONTENTS);
    udp.endPacket();

    // Start reply wait timer
    lastTime = millis();
    waitingForReplies = true;
}

static void handleJS() {
    String message = F(JS_FILE);
    server.send(200, "text/javascript", message);
}

static void handleCSS() {
    String message = F(CSS_FILE);
    server.send(200, "text/css", message);
}

static void handleFavicon() {
    server.send_P(200, faviconMimeType, (PGM_P)favicon, faviconSize);
}

static void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++){
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

void setup(void) {
    Serial.begin(DEBUG_BAUDRATE);

#ifdef DEBUG
    Serial.println();
    Serial.println("ESP-Weather init...");
#endif // DEBUG

    //sensor.begin();
    // The SHT library is simpy calling Wire.begin(), but the default
    // config does not match the pins i'm using (sda - 2; scl - 0)
    Wire.begin(2, 0);

    // Here you can override the WiFiManager configuration. Leave
    // the default autoConnect in use for the default behaviour.
    // To force the config portal, even though the module was connected before,
    // comment-out autoConnect and use startConfigPortal instead.
    WiFiManager wifiManager;
    wifiManager.autoConnect(DEFAULT_SSID, DEFAULT_PASS);
    //wifiManager.startConfigPortal(DEFAULT_SSID, DEFAULT_PASS);

    initMemory();
    storage = readMemory();

    // Wait for connection
    if (WiFi.status() != WL_CONNECTED) {
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
#ifdef DEBUG
            Serial.print(".");
#endif // DEBUG
        }
#ifdef DEBUG
        Serial.println("");
#endif // DEBUG
    }

#ifdef DEBUG
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif // DEBUG

    broadcastIP = ~WiFi.subnetMask() | WiFi.gatewayIP();

    server.on("/", handleRoot);
    server.on("/index.html", handleRoot);
    server.on("/view.js", handleJS);
    server.on("/style.css", handleCSS);
    server.on("/favicon.ico", handleFavicon);
    server.onNotFound(handleNotFound);
    server.begin();
    serverSocket.begin();
    ntpInit();
    udp.begin(BROADCAST_PORT);

#ifdef DEBUG
    Serial.println("ESP-Weather ready!");
#endif // DEBUG
}

void loop(void) {
    ntpRun();
    server.handleClient();

    // Websocket fuer Browser
    WiFiClient client = serverSocket.available();
    if (client.connected() && webSocketServer.handshake(client)) {
        Serial.println("Building WebSocket Response...");

        String json = "{\"H\":";
        json += String(sensor.getHumidity());
        json += ",";
        json += "\"T\":";
        json += String(sensor.getTemperature());
        json += ", \"EEPROM\" : [";
        for (int i = 0; i < storage.header.count; i++) {
            json += "{\"H\":";
            json += String(storage.data[i].humidity);
            json += ",";
            json += "\"T\":";
            json += String(storage.data[i].temperature);
            json += "}";
            if (i < storage.header.count - 1) {
                json += ",";
            }
        }
        json += "]}";

#ifdef DEBUG
        Serial.println("WebSocket Response:");
        Serial.println(json);
#endif // DEBUG

        webSocketServer.sendData(json);
        client.flush();
        client.stop();
    }

    // EEPROM-Schreiben jede Stunde
    if ((((((millis() - timeReceived) / 1000) + timestamp) % 3600) == 0)
            && (timestamp != 0) && (((millis() - lastStorageTime) > 100000) || storeAtBoot) ) {
#ifdef DEBUG
        Serial.println("Storing new data packet...");
#endif // DEBUG
        lastStorageTime = millis();
        storeAtBoot = 0;
        if (storage.header.count < MAX_STORAGE) {
            storage.header.count++;
        } else {
            for(int i = 0; i < MAX_STORAGE - 1; i++) {
                storage.data[i] = storage.data[i+1];
            }
        }
        storage.data[storage.header.count - 1].temperature = sensor.getTemperature();
        storage.data[storage.header.count - 1].humidity = sensor.getHumidity();
        writeMemory(storage);
    }

    // UDP
    int packetSize = udp.parsePacket();
    if (packetSize) {
        IPAddress remoteIp = udp.remoteIP();
        // read the packet into packetBufffer
        int len = udp.read(packetBuffer, UDP_PACKET_BUFFER_SIZE);
        if (len > 0) {
            packetBuffer[len] = 0;
        }

#ifdef DEBUG
        Serial.print("Got UDP packet: ");
        Serial.println(packetBuffer);
#endif // DEBUG

        if (strcmp(packetBuffer, UDP_PING_CONTENTS) == 0) {
#ifdef DEBUG
            Serial.println("Broadcast");
#endif // DEBUG
            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            udp.print(UDP_ECHO_CONTENTS);
            udp.endPacket();
        } else if((strcmp(packetBuffer, UDP_ECHO_CONTENTS) == 0) && (waitingForReplies == true)) {
            vecClients.push_back(udp.remoteIP());
        }
    }

    if (((millis() - lastTime) >= MAX_BROADCAST_WAIT_TIME) && (waitingForReplies == true)) {
#ifdef DEBUG
        Serial.println("Timeout, sending response...");
#endif // DEBUG

        waitingForReplies = false;
        String message = F(HTML_BEGIN);
        message += "var clients = Array(";
        message += "\"" + WiFi.localIP().toString() + "\"";
        if (vecClients.size() > 0) {
            message += ", ";
        }
        for (int i = 0; i < vecClients.size(); i++) {
            message += "\"" + vecClients[i].toString() + "\"";
            if (i < (vecClients.size() - 1)) {
                message += ", ";
            }
        }
        message += ");";
        message += F(HTML_END);

        vecClients.clear();

        server.send(200, "text/html", message);
    }
}

