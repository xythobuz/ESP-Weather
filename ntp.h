#ifndef __NTP_H__
#define __NTP_H__

extern unsigned long timestamp; // received epoch time, 0 until valid
extern unsigned long timeReceived; // systick matching received epoch, 0 until valid

void ntpInit(void);
void ntpRun(void);

#endif // __NTP_H__

