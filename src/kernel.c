
#include <stdint.h>
#include <stdbool.h>
#include "gdt.c"
#include "header/kernel-entrypoint.h"
#include "header/text/framebuffer.h"
#include "header/interrupt/interrupt.h"
#include "header/interrupt/idt.h"

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    framebuffer_clear();
    // framebuffer_write(0,0,' ',0xF,0xF);
    framebuffer_set_cursor(0, 0);
    __asm__("int $0xF4");
    while (true);
}