#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/text/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"

// void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
// {
// 	out(0x3D4, 0x0A);
// 	out(0x3D5, (in(0x3D5) & 0xC0) | cursor_start);
 
// 	out(0x3D4, 0x0B);
// 	out(0x3D5, (in(0x3D5) & 0xE0) | cursor_end);
// }

void framebuffer_set_cursor(uint8_t x, uint8_t y) {
    // TODO : Implement
    uint16_t pos = x * 80 + y;
 
	out(CURSOR_PORT_CMD, 0x0F);
	out(CURSOR_PORT_DATA, (uint8_t) (pos & 0xFF));
	out(CURSOR_PORT_CMD, 0x0E);
	out(CURSOR_PORT_DATA, (uint8_t) ((pos >> 8) & 0xFF));
}

void framebuffer_write(uint8_t row, uint8_t col, char c, uint8_t fg, uint8_t bg) {
    uint16_t attrib = (bg << 4) | (fg & 0x0F);
    volatile uint16_t * where;
    where = (volatile uint16_t *)0xB8000 + (row * 80 + col) ;
    *where = c | (attrib << 8);
    // TODO : Implement
}

// uint16_t get_cursor_position(void)
// {
//     uint16_t pos = 0;
//     outb(0x3D4, 0x0F);
//     pos |= inb(0x3D5);
//     outb(0x3D4, 0x0E);
//     pos |= ((uint16_t)inb(0x3D5)) << 8;
//     return pos;
// }

void framebuffer_clear(void) {
    // TODO : Implement
    volatile uint16_t *fb = (volatile uint16_t *)0xB8000;
    memset((void *)fb, 0x00, 80 * 25 * 2);
}