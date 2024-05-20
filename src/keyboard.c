#include "header/driver/keyboard.h"
#include "header/driver/framebuffer.h"
#include "header/cpu/portio.h"
#include "header/stdlib/string.h"
#include <stdbool.h>

struct KeyboardDriverState keyboard_driver_state;
char buff[KEYBOARD_BUFFER_SIZE];
uint8_t row_now = 0;
uint8_t col_now = 0;
uint8_t l = 0;

const char keyboard_scancode_1_to_ascii_map[256] = {
      0, 0x1B, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0',  '-', '=', '\b', '\t',
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']', '\n',   0,  'a',  's',
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0, '\\',  'z', 'x',  'c',  'v',
    'b',  'n', 'm', ',', '.', '/',   0, '*',    0, ' ',   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0, '-',    0,    0,   0,  '+',    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,

      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
      0,    0,   0,   0,   0,   0,   0,   0,    0,   0,   0,    0,    0,   0,    0,    0,
};

void keyboard_state_activate(void) {
    keyboard_driver_state.keyboard_input_on = true;
    keyboard_driver_state.keyboard_input = '\0';
    // for (uint32_t i = 0; i < KEYBOARD_BUFFER_SIZE; i++) {
    //     keyboard_driver_state.keyboard_buffer[i] = '\0';
    // }
    // keyboard_driver_state.buffer_index = 0;
    // for (uint32_t i = 0; i < KEYBOARD_BUFFER_SIZE; i++) {
    //     buff[i] = '\0';
    // }
}

void keyboard_state_deactivate(void) {
    keyboard_driver_state.keyboard_input_on = false;
}

void get_keyboard_buffer(char *buf) {
    *buf = keyboard_driver_state.keyboard_input;
    keyboard_driver_state.keyboard_input = '\0';
}

bool is_keyboard_blocking(void) {
    return keyboard_driver_state.keyboard_input_on;
}

void reset_write_position() {
    row_now = 0;
    col_now = 0;
}

void keyboard_isr(void) {
    uint8_t scancode = in(KEYBOARD_DATA_PORT);
    if (scancode & 0x80) {
        // Break code
    }
    else if (keyboard_driver_state.keyboard_input_on) {
        char ascii_char = keyboard_scancode_1_to_ascii_map[scancode];
        keyboard_driver_state.keyboard_input = ascii_char;
        if (ascii_char == '\n') {
            putchar(ascii_char, 0x00);
            keyboard_state_deactivate();
        }
        else if (ascii_char == '\b') {
            if (l > 0) {
                framebuffer_write(row_now, col_now, ' ', 0x0, 0x0);
                framebuffer_set_cursor(row_now, col_now);
                col_now--;
                l--;
            }
        } else if (ascii_char == '\t') {
            for (int i = 0; i < 2; i++) {
                framebuffer_write(row_now,col_now,' ', 0x0, 0x0);
                col_now++; 
                l++;
            }
        } else if (ascii_char != 0 && l < KEYBOARD_BUFFER_SIZE - 1)  {
            putchar(ascii_char, 0xF);
            l++;
        }
        putchar('\0', 0xF);
    }
    pic_ack(IRQ_KEYBOARD);
}

void putchar(char s, uint32_t color) {
    if (s == '\n') {
        row_now++;
        col_now = 0;
        l = 0;
    } else if (s != 0) {
        framebuffer_write(row_now, col_now, s, color, 0x00);
        col_now++;
    }
    framebuffer_write(row_now, col_now, '\0', 0xF, 0);
    framebuffer_set_cursor(row_now, col_now);
}

void puts(char* s, uint32_t len, uint32_t color) {
    for (uint32_t i = 0; i < len; i++) {
        putchar(*(s + i), color);
    }
}