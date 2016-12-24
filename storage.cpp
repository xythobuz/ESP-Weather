#include <Arduino.h>
#include <EEPROM.h>
#include "storage.h"

void initMemory(void) {
    EEPROM.begin(EEPROM_SIZE);
}

void writeMemory(PersistentStorage &s) {
    Serial.println("write Memory");
    unsigned char* r = (unsigned char*) &s;
    uint16_t a = 0, b = 0;
    for (int i = 0; i < sizeof(PersistentStorage) - 2; i++) {
        a = (a + r[i]) % 255;
        b = (b + a) % 255;
    }
    s.header.checksum = (b << 8) | a;
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
        Serial.print("Checksum error ");
        Serial.print(s.header.checksum);
        Serial.print(" ");
        Serial.println((b << 8) | a);
        s.header.count = 0;
    } else {
        Serial.println("Checksum ok");
    }
    return s;
}

