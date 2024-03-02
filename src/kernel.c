#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/cpu/gdt.c"
#include "header/kernel-entrypoint.h"

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    //  __asm__ volatile("int $1");
    //  int a =0;
    // while (true) a += 1;
    while (true);
}