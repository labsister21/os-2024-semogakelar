
#include <stdint.h>
#include <stdbool.h>
#include "gdt.c"
#include "header/kernel-entrypoint.h"
#include "header/driver/framebuffer.h"
#include "header/driver/keyboard.h"
#include "header/cpu/interrupt.h"
#include "header/driver/disk.h"
#include "header/interrupt/idt.h"

void kernel_setup(void) {
    // load_gdt(&_gdt_gdtr);
    // pic_remap();
    // initialize_idt();
    // activate_keyboard_interrupt();
    // framebuffer_clear();
    // framebuffer_set_cursor(0, 0);
    // __asm__("int $0x4");
        
    // int col = 0;
    // keyboard_state_activate();
    // while (true) {
    //      char c;
    //      get_keyboard_buffer(&c);
    //      if (c) {
    //         framebuffer_write(0, col++, c, 0xF, 0);
    //         framebuffer_write(0, col, '\0', 0xF, 0);
    //         framebuffer_set_cursor(0, col);
    //      }
    // }
    load_gdt(&_gdt_gdtr);
    pic_remap();
    activate_keyboard_interrupt();
    initialize_idt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);

    struct BlockBuffer b;
    for (int i = 0; i < 512; i++) b.buf[i] = i % 16;
    write_blocks(&b, 17, 1);
    while (true);
}