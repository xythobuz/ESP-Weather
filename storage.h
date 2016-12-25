/*
 * storage.h
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

