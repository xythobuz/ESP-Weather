#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "config.h"
#include "ntp.h"

//#define DEBUG

IPAddress timeServerIP;
byte ntpPacketBuffer[NTP_PACKET_SIZE];
WiFiUDP ntp;

unsigned long lastNTP = 0; // timer for retries
unsigned long timestamp = 0; // received epoch time
unsigned long timeReceived = 0; // systick matching received epoch

static void sendNTPpacket(IPAddress& address) {
#ifdef DEBUG
    Serial.println("Sending NTP packet...");
#endif

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

void ntpInit(void) {
    ntp.begin(NTP_PORT_FROM);
    WiFi.hostByName(NTP_SERVER_NAME, timeServerIP);
    lastNTP = millis();
    sendNTPpacket(timeServerIP);
}

void ntpRun(void) {
    // Retry if we haven't received an answer soon enough
    if ((timestamp == 0) && ((millis() - lastNTP) > NTP_RETRY_TIMEOUT)) {
#ifdef DEBUG
        Serial.println("NTP packet retry...");
#endif // DEBUG

        WiFi.hostByName(NTP_SERVER_NAME, timeServerIP);
        lastNTP = millis();
        sendNTPpacket(timeServerIP);
    }

    // Process received response
    if (ntp.parsePacket() >= NTP_PACKET_SIZE) {
        ntp.read(ntpPacketBuffer, NTP_PACKET_SIZE);
        unsigned long highWord = word(ntpPacketBuffer[40], ntpPacketBuffer[41]);
        unsigned long lowWord = word(ntpPacketBuffer[42], ntpPacketBuffer[43]);
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        const unsigned long seventyYears = 2208988800UL;
        unsigned long epoch = secsSince1900 - seventyYears;
        timestamp = epoch;
        timeReceived = millis();

#ifdef DEBUG
        Serial.print("Got NTP time: ");
        Serial.println(epoch);
#endif // DEBUG
    }
}

