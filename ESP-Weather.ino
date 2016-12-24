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
#include "storage.h"

#define WEB_PORT 80
#define BROADCAST_PORT 2390
#define WEBSOCKET_PORT 2391
#define NTP_PORT_FROM 2392
#define NTP_PORT_TO 123

#define DEFAULT_SSID "ESP-Weather"
#define DEFAULT_PASS "testtest"

#define MAX_BROADCAST_WAIT_TIME 550
#define NTP_RETRY_TIMEOUT 2000

// NTP-Client
IPAddress timeServerIP; 
const char* ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; 
byte ntpPacketBuffer[NTP_PACKET_SIZE]; 
WiFiUDP ntp;
unsigned long timestamp = 0;
unsigned long timeReceived = 0;
unsigned long lastStorageTime = 0;
byte storeAtBoot = 1;
unsigned long lastNTP = 0;

SHT21 SHT21;
ESP8266WebServer server(WEB_PORT);
WiFiServer serverSocket(WEBSOCKET_PORT);
WebSocketServer webSocketServer;

IPAddress broadcastIP;

PersistentStorage storage;

std::vector<IPAddress> vecClients; 

// UDP-Config
#define UDP_PACKET_BUFFER_SIZE 255
char packetBuffer[UDP_PACKET_BUFFER_SIZE];
const char pingBuffer[] = "pingESP8266v0.1";
const char echoBuffer[] = "echoESP8266v0.1";
WiFiUDP udp;

unsigned long lastTime;
bool waitingForReplies = false;

// Using the RawGit.com service to serve the scripts directly from GitHub.
// Consider using cdn.rawgit.com to reduce their server load.
const char* htmlBegin = "<html><head>\
<title>Sysadmin</title>\
<script src=\"https://rawgit.com/xythobuz/ESP-Weather/master/static/jquery-3.1.1.min.js\"></script>\
<script src=\"https://rawgit.com/xythobuz/ESP-Weather/master/static/bootstrap.min.js\"></script>\
<script src=\"https://rawgit.com/xythobuz/ESP-Weather/master/static/Chart.bundle.min.js\"></script>\
<script src=\"https://rawgit.com/xythobuz/ESP-Weather/master/static/script.js\"></script>\
<link rel=\"stylesheet\" href=\"https://rawgit.com/xythobuz/ESP-Weather/master/static/bootstrap.min.css\" />\
</head><body>\
<script type=\"text/javascript\">";
const char* htmlEnd = "</script></body></html>";

void handleRoot() {
    Serial.println("Sending UDP Broadcast...");

    // Send UDP broadcast to other modules
    udp.beginPacket(broadcastIP, BROADCAST_PORT);
    udp.write(pingBuffer);
    udp.endPacket();

    // Start reply wait timer
    lastTime = millis();
    waitingForReplies = true;
}

void handleNotFound(){
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
    // Debugging
    Serial.begin(115200);
    Serial.println();
    Serial.println("ESP-Weather init...");

    //SHT21.begin();
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
            Serial.print(".");
        }
        Serial.println("");
    }

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    broadcastIP = ~WiFi.subnetMask() | WiFi.gatewayIP();

    server.on("/", handleRoot);
    server.onNotFound(handleNotFound);
    server.begin();

    serverSocket.begin();

    // NTP-Client
    ntp.begin(NTP_PORT_FROM);
    WiFi.hostByName(ntpServerName, timeServerIP); 
    lastNTP = millis();
    sendNTPpacket(timeServerIP); 

    udp.begin(BROADCAST_PORT);
    Serial.println("ESP-Weather ready!");
}

void loop(void){
    server.handleClient();

    // Websocket fuer Browser
    WiFiClient client = serverSocket.available();
    if (client.connected() && webSocketServer.handshake(client)) {
        Serial.println("Building WebSocket Response...");

        String json = "{\"H\":";
        json += String(SHT21.getHumidity());
        json += ",";
        json += "\"T\":";
        json += String(SHT21.getTemperature());
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

        Serial.println("WebSocket Response:");
        Serial.println(json);

        webSocketServer.sendData(json);
        client.flush();
        client.stop();
    }

    // NTP wiederholen falls keine Antwort
    if ((timestamp == 0) && ((millis() - lastNTP) > NTP_RETRY_TIMEOUT)) {
        Serial.println("NTP packet retry...");
        WiFi.hostByName(ntpServerName, timeServerIP);
        lastNTP = millis();
        sendNTPpacket(timeServerIP);
    }

    // NTP Paket vom Server erhalten
    if (ntp.parsePacket() >= NTP_PACKET_SIZE) {
        ntp.read(ntpPacketBuffer, NTP_PACKET_SIZE);
        unsigned long highWord = word(ntpPacketBuffer[40], ntpPacketBuffer[41]);
        unsigned long lowWord = word(ntpPacketBuffer[42], ntpPacketBuffer[43]);
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        const unsigned long seventyYears = 2208988800UL;
        unsigned long epoch = secsSince1900 - seventyYears;
        timestamp = epoch;
        timeReceived = millis();
        Serial.print("Got NTP time: ");
        Serial.println(epoch);
    }

    // EEPROM-Schreiben jede Stunde
    if ((((((millis() - timeReceived) / 1000) + timestamp) % 3600) == 0)
            && (timestamp != 0) && (((millis() - lastStorageTime) > 100000) || storeAtBoot) ) {
        Serial.println("Storing new data packet...");
        lastStorageTime = millis();
        storeAtBoot = 0;
        if (storage.header.count < MAX_STORAGE) {
            storage.header.count++;
        } else {
            for(int i = 0; i < MAX_STORAGE - 1; i++) {
                storage.data[i] = storage.data[i+1];
            }
        }
        storage.data[storage.header.count - 1].temperature = SHT21.getTemperature();
        storage.data[storage.header.count - 1].humidity = SHT21.getHumidity();
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

        Serial.print("Got UDP packet: ");
        Serial.println(packetBuffer);

        if (strcmp(packetBuffer, pingBuffer) == 0) {
            Serial.println("Broadcast");
            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            udp.print(echoBuffer);
            udp.endPacket();
        } else if((strcmp(packetBuffer, echoBuffer) == 0) && (waitingForReplies == true)) {
            vecClients.push_back(udp.remoteIP());
        }
    }

    if (((millis() - lastTime) >= MAX_BROADCAST_WAIT_TIME) && (waitingForReplies == true)) {
        Serial.println("Timeout, sending response...");
        waitingForReplies = false;
        String message = htmlBegin;
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
        message += htmlEnd;

        vecClients.clear();

        server.send(200, "text/html", message);
    }
}

void sendNTPpacket(IPAddress& address) {
    Serial.println("Sending NTP packet...");
    memset(ntpPacketBuffer, 0, NTP_PACKET_SIZE);
    ntpPacketBuffer[0] = 0b11100011; // LI, Version, Mode
    ntpPacketBuffer[1] = 0;
    ntpPacketBuffer[2] = 6;
    ntpPacketBuffer[3] = 0xEC;
    ntpPacketBuffer[12]  = 49;
    ntpPacketBuffer[13]  = 0x4E;
    ntpPacketBuffer[14]  = 49;
    ntpPacketBuffer[15]  = 52;

    ntp.beginPacket(address, NTP_PORT_TO);
    ntp.write(ntpPacketBuffer, NTP_PACKET_SIZE);
    ntp.endPacket();
}

