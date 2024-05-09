#ifndef _USER_SHELL_H
#define _USER_SHELL_H

#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"

// Color macros
#define LIGHT_GREEN 0b1010
#define GREY        0b0111
#define DARK_GREY   0b1000
#define LIGHT_BLUE  0b1001
#define RED         0b1100
#define WHITE       0b1111
#define BLACK       0b0000

// Current directory number & dir table
extern uint32_t current_directory;
extern struct FAT32DirectoryTable current_dir_table;

// Run a system call to the kernel
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

// Wrapper function for printing strings to screen
void put(char* str, uint8_t color);

// Parse path string into array of file/folder names
void parse_path(char* str, char paths[][512], uint32_t* num_of_directory);

// Print current directory to screen
void print_current_directory();

#endif

