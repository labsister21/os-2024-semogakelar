#include <stdint.h>
#include <stdio.h>
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
#include "header/memory/paging.h"

// void kernel_setup(void) {
//     load_gdt(&_gdt_gdtr);
//     pic_remap();
//     initialize_idt();
//     activate_keyboard_interrupt();
//     framebuffer_clear();
//     framebuffer_set_cursor(0, 0);
//     initialize_filesystem_fat32();

//     /* ========== CRUD ========== */
    
//     struct ClusterBuffer b[5];
//     int8_t ret_val;

//     struct FAT32DriverRequest request1 = {
//         .buf                    = &b,
//         .name                   = "folder1\0",
//         .ext                    = "\0\0\0",
//         .parent_cluster_number  = ROOT_CLUSTER_NUMBER,
//         .buffer_size            = 10000
//     };
//     ret_val = read_directory(request1);  // ret_val = 0
//     ret_val = read(request1);            // ret_val = 1;

//     struct FAT32DriverRequest request2 = {
//         .buf                    = &b,
//         .name                   = "daijoubu",
//         .ext                    = "\0\0\0",
//         .parent_cluster_number  = 0x04,
//         .buffer_size            = 10000
//     };
//     ret_val = read_directory(request2);  // ret_val = 1
//     ret_val = read(request2);            // ret_val = 0

//     request2.buffer_size = 0;
//     ret_val = read_directory(request2);  // ret_val = -1
//     ret_val = read_directory(request2);  // ret_val = -1

//     struct FAT32DriverRequest request3 = {
//         .buf                    = &b,
//         .name                   = "gaada\0\0\0",
//         .ext                    = "\0\0\0",
//         .parent_cluster_number  = 0x04,
//         .buffer_size            = 10000
//     };
//     ret_val = read_directory(request3);  // ret_val = 2
//     ret_val = read_directory(request3);  // ret_val = 2
    
//     struct FAT32DriverRequest request4 = {
//         .buf                    = &b,
//         .name                   = "imfine2\0",
//         .ext                    = "\0\0\0",
//         .parent_cluster_number  = ROOT_CLUSTER_NUMBER,
//         .buffer_size            = 6000
//     };
//     ret_val = write(request4);  // ret_val = 0
//     ret_val = write(request4);  // ret_val = 1

//     request4.parent_cluster_number = 0x05;
//     ret_val = write(request4);  // ret_val = 2

//     struct FAT32DriverRequest request5 = {
//         .buf                    = &b,
//         .name                   = "smgkelar",
//         .ext                    = "\0\0\0",
//         .parent_cluster_number  = ROOT_CLUSTER_NUMBER,
//         .buffer_size            = 0
//     };
//     ret_val = write(request5);   // ret_val = 0
//     ret_val = Delete(request5);  // ret_val = 0
//     ret_val = Delete(request5);  // ret_val = 1
//     ret_val = Delete(request4);  // ret_val = -1
//     request4.parent_cluster_number = ROOT_CLUSTER_NUMBER;
//     ret_val = Delete(request4);  // ret_val = 0
//     ret_val = Delete(request1);  // ret_val = 2

//     ret_val++; ret_val--;


//     /* ========== Keyboard ========== */

//     keyboard_state_activate();
    
//     int row = 0, col = 0;
//     while (true) {
//         char c; get_keyboard_buffer(&c);

//         if (c == '\n') row++, col = 0;
//         else if (c == '\b') {
//             if (col > 0) col--; else row--, col = 80;
//             while (*(FRAMEBUFFER_MEMORY_OFFSET + (row*80 + col)*2) == '\0') {
//                 if (col > 0) col--;
//                 else row--, col = 80;
//             }
//         }
//         else if (c != 0) framebuffer_write(row, col++, c, 0xF, 0);

//         framebuffer_write(row, col, '\0', 0xF, 0);
//         framebuffer_set_cursor(row, col);
//     }
// }

void kernel_setup(void) {
    load_gdt(&_gdt_gdtr);
    pic_remap();
    initialize_idt();
    activate_keyboard_interrupt();
    framebuffer_clear();
    framebuffer_set_cursor(0, 0);
    initialize_filesystem_fat32();
    gdt_install_tss();
    set_tss_register();

    // Allocate first 4 MiB virtual memory
    paging_allocate_user_page_frame(&_paging_kernel_page_directory, (uint8_t*) 0);

    // Write shell into memory
    struct FAT32DriverRequest request = {
        .buf                   = (uint8_t*) 0,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 0x100000,
    };
    int8_t ret_val = read(request);


    // Set TSS $esp pointer and jump into shell 
    set_tss_kernel_current_stack();
    kernel_execute_user_program((uint8_t*) 0);

    ret_val++; ret_val--;
    while (true);
}
