#include <stdint.h>
#include <stdbool.h>
#include "header/cpu/gdt.h"
#include "header/cpu/idt.h"
#include "header/cpu/interrupt.h"
#include "header/driver/keyboard.h"
#include "header/kernel-entrypoint.h"
#include "header/driver/framebuffer.h"
#include "header/driver/disk.h"
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();

    struct ClusterBuffer b[5];
    int8_t ret_val;

    struct FAT32DriverRequest request1 = {
        .buf                    = &b,
        .name                   = "folder1\0",
        .ext                    = "\0\0\0",
        .parent_cluster_number  = ROOT_CLUSTER_NUMBER,
        .buffer_size            = 0x0
    };
    ret_val = read_directory(request1);

    struct FAT32DriverRequest request2 = {
        .buf                    = &b,
        .name                   = "daijoubu",
        .ext                    = "\0\0\0",
        .parent_cluster_number  = 0x04,
        .buffer_size            = 10000
    };
    ret_val = read(request2);

    struct FAT32DriverRequest request3 = {
        .buf                    = &b,
        .name                   = "imfine2\0",
        .ext                    = "\0\0\0",
        .parent_cluster_number  = ROOT_CLUSTER_NUMBER,
        .buffer_size            = 7000
    };
    ret_val = write(request3);

    struct FAT32DriverRequest request4 = {
        .buf                    = &b,
        .name                   = "smgkelar",
        .ext                    = "\0\0\0",
        .parent_cluster_number  = ROOT_CLUSTER_NUMBER,
        .buffer_size            = 0
    };
    ret_val = write(request4);

    ret_val++; ret_val--;
    while (true);
     
    // keyboard_state_activate();
    
    // int row = 0, col = 0;
    // while (true) {
    //     char c; get_keyboard_buffer(&c);

    //     if (c == '\n') row++, col = 0;
    //     else if (c == '\b') {
    //         if (col > 0) col--; else row--, col = 80;
    //         while (*(FRAMEBUFFER_MEMORY_OFFSET + (row*80 + col)*2) == '\0') {
    //             if (col > 0) col--;
    //             else row--, col = 80;
    //         }
    //     }
    //     else if (c != 0) framebuffer_write(row, col++, c, 0xF, 0);

    //     framebuffer_write(row, col, '\0', 0xF, 0);
    //     framebuffer_set_cursor(row, col);
    // }
}

