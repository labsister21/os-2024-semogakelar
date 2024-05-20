#include "header/cmos/cmos.h"
#include "header/stdlib/string.h"
#include "header/driver/framebuffer.h"

int century_register = 0x00;
unsigned char second;
unsigned char minute;
unsigned char hour;
unsigned char day;
unsigned char month;
unsigned int year;


int get_update_in_progress_flag() {
      out(CMOS_ADDRESS, 0x0A);
      return (in(CMOS_DATA) & 0x80);
}
 
unsigned char get_RTC_register(int reg) {
      out(CMOS_ADDRESS, reg);
      return in(CMOS_DATA);
}
 
void read_rtc() {
      // local variable to ensure the validity and persistence of date and time gotten
      unsigned char century;
      unsigned char last_second;
      unsigned char last_minute;
      unsigned char last_hour;
      unsigned char last_day;
      unsigned char last_month;
      unsigned char last_year;
      unsigned char last_century;
      unsigned char registerB;
 
      while (get_update_in_progress_flag());     
      second = get_RTC_register(0x00);
      minute = get_RTC_register(0x02);
      hour = get_RTC_register(0x04);
      day = get_RTC_register(0x07);
      month = get_RTC_register(0x08);
      year = get_RTC_register(0x09);

      if(century_register != 0) {
            century = get_RTC_register(century_register);
      }
 
      do {
            last_second = second;
            last_minute = minute;
            last_hour = hour;
            last_day = day;
            last_month = month;
            last_year = year;
            last_century = century;
 
            while (get_update_in_progress_flag());  
            second = get_RTC_register(0x00);
            minute = get_RTC_register(0x02);
            hour = get_RTC_register(0x04);
            day = get_RTC_register(0x07);
            month = get_RTC_register(0x08);
            year = get_RTC_register(0x09);

            if(century_register != 0) {
                  century = get_RTC_register(century_register);
            }

      } while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
               (last_day != day) || (last_month != month) || (last_year != year) ||
               (last_century != century) );
 
      registerB = get_RTC_register(0x0B);
 
      // Convert BCD to binary values
 
      if (!(registerB & 0x04)) {
            second = (second & 0x0F) + ((second / 16) * 10);
            minute = (minute & 0x0F) + ((minute / 16) * 10);
            hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
            day = (day & 0x0F) + ((day / 16) * 10);
            month = (month & 0x0F) + ((month / 16) * 10);
            year = (year & 0x0F) + ((year / 16) * 10);
            if(century_register != 0) {
                  century = (century & 0x0F) + ((century / 16) * 10);
            }
      }
 
      // Convert 12 hour clock to 24 hour clock and add 7 to synchronize with WIB
      if (!(registerB & 0x02) && (hour & 0x80)) {
            hour = ((hour & 0x7F) + 12) % 24;
      }
 
      // Calculate the full (4-digit) year
      if(century_register != 0) {
            year += century * 100;
      } else {
            year += (CURRENT_YEAR / 100) * 100;
            if(year < CURRENT_YEAR) year += 100;
      }
}

// Returns a string containing current time, eg. "22/08/2024 13:23:59"
char* get_current_time() {
    return NULL;
}

void print_current_time() {
    read_rtc();
    char* current_hour = itoa((((int) hour) + 7) % 24, 10);
    if (strlen(current_hour) == 1){
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 8, '0', 0b1111, 0);
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 7, current_hour[0], 0b1111, 0);
    } else {
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 8, (current_hour[0] == 0)? '0' : current_hour[0], 0b1111, 0);
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 7, (current_hour[1] == 0)? '0' : current_hour[1], 0b1111, 0);
    }

    framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 6, ':', 0b1111, 0);

    char* current_minute = itoa(((int) minute), 10);
    if (strlen(current_minute) == 1){
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 5, '0', 0b1111, 0);
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 4, current_minute[0], 0b1111, 0);
    } else {
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 5, (current_minute[0] == 0)? '0' : current_minute[0], 0b1111, 0);
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 4, (current_minute[1] == 0)? '0' : current_minute[1], 0b1111, 0);
    }

    framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 3, ':', 0b1111, 0);

    char* current_second = itoa(((int) second), 10);
    if (strlen(current_second) == 1){
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 2, '0', 0b1111, 0);
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, current_second[0], 0b1111, 0);
    } else {
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 2, (current_second[0] == 0)? '0' : current_second[0], 0b1111, 0);
      framebuffer_write(SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, (current_second[1] == 0)? '0' : current_second[1], 0b1111, 0);
    }
}