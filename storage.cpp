/*
 * storage.cpp
 *
 * EEPROM data history storage.
 *
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <xythobuz@xythobuz.de> & <ghost-ghost@web.de> wrote this file.  As long as
 * you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer
 * in return.                                   Thomas Buck & Christian Högerle
 * ----------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <EEPROM.h>
#include "storage.h"

//#define DEBUG

void initMemory(void) {
    EEPROM.begin(EEPROM_SIZE);
}

void writeMemory(PersistentStorage &s) {
    unsigned char* r = (unsigned char*) &s;
    uint16_t a = 0, b = 0;
    for (int i = 0; i < sizeof(PersistentStorage) - 2; i++) {
        a = (a + r[i]) % 255;
        b = (b + a) % 255;
    }
    s.header.checksum = (b << 8) | a;

#ifdef DEBUG
    Serial.print("write memory: ");
    Serial.println(s.header.checksum);
#endif // DEBUG

    for (int i = 0; i < sizeof(PersistentStorage); i++) {
        EEPROM.write(i, r[i]);
    }
    EEPROM.commit();
}

PersistentStorage readMemory() {
    PersistentStorage s;
    unsigned char* r = (unsigned char*) &s;
    for (int i = 0; i < sizeof(PersistentStorage); i++) {
        r[i] = EEPROM.read(i);
    }
    uint16_t a = 0, b = 0;
    for (int i = 0; i < sizeof(PersistentStorage) - 2; i++) {
        a = (a + r[i]) % 255;
        b = (b + a) % 255;
    }
    if (s.header.checksum != ((b << 8) | a)) {
#ifdef DEBUG
        Serial.print("read memory: checksum error: ");
        Serial.print(s.header.checksum);
        Serial.print(" != ");
        Serial.println((b << 8) | a);
#endif // DEBUG
        s.header.count = 0;
    } else {
#ifdef DEBUG
        Serial.println("read memory: checksum ok");
#endif // DEBUG
    }
    return s;
}

