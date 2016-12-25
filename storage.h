#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "config.h"

struct __attribute__((__packed__)) Measurement {
    float temperature;
    float humidity;
};

struct __attribute__((__packed__)) Header {
    uint16_t count;
    uint16_t checksum;
};

#define MAX_STORAGE (EEPROM_SIZE - sizeof(Header)) / sizeof(Measurement)

struct __attribute__((__packed__)) PersistentStorage {
    Measurement data[MAX_STORAGE];
    Header header;
};

void initMemory(void);
void writeMemory(PersistentStorage &s);
PersistentStorage readMemory();

#endif // __STORAGE_H__

