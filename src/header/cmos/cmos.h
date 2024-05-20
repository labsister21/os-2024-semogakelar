#ifndef _CMOS_H
#define _CMOS_H

#include "header/cpu/portio.h"

#define CURRENT_YEAR        2024
#define CMOS_ADDRESS        0x70
#define CMOS_DATA           0x71

// Current time, periodically updated
extern unsigned char second;
extern unsigned char minute;
extern unsigned char hour;
extern unsigned char day;
extern unsigned char month;
extern unsigned int year;

int get_update_in_progress_flag();

unsigned char get_RTC_register(int reg);

void read_rtc();

void print_current_time();

#endif