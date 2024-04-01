#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/cpu/idt.h"
#include "header/cpu/interrupt.h"
#include "header/driver/keyboard.h"
#include "header/kernel-entrypoint.h"
#include "header/driver/framebuffer.h"

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
        
    keyboard_state_activate();
    
    int row = 0, col = 0;
    while (true) {
        char c; get_keyboard_buffer(&c);

        if (c == '\n') row++, col = 0;
        else if (c == '\b') {
            if (col > 0) col--; else row--, col = 80;
            while (*(FRAMEBUFFER_MEMORY_OFFSET + (row*80 + col)*2) == '\0') {
                if (col > 0) col--;
                else row--, col = 80;
            }
        }
        else if (c != 0) framebuffer_write(row, col++, c, 0xF, 0);

        framebuffer_write(row, col, '\0', 0xF, 0);
        framebuffer_set_cursor(row, col);
    }
}
