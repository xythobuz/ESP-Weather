/*
 * ntp.h
 *
 * Simple NTP implementation to aquire the current time matched to the systick.
 *
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <xythobuz@xythobuz.de> & <ghost-ghost@web.de> wrote this file.  As long as
 * you retain this notice you can do whatever you want with this stuff. If we
 * meet some day, and you think this stuff is worth it, you can buy us a beer
 * in return.                                   Thomas Buck & Christian Högerle
 * ----------------------------------------------------------------------------
 */

#ifndef __NTP_H__
#define __NTP_H__

extern unsigned long timestamp; // received epoch time, 0 until valid
extern unsigned long timeReceived; // systick matching received epoch, 0 until valid

void ntpInit(void);
void ntpRun(void);

#endif // __NTP_H__

