#include <stdint.h>
#include <stdbool.h>
#include "gdt.c"
#include "header/kernel-entrypoint.h"

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    while (true);
}