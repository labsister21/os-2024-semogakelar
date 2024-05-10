#ifndef _USER_SHELL_H
#define _USER_SHELL_H

#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"

// Color macros
#define BLACK           0b0000
#define BLUE            0b0001
#define GREEN           0b0010
#define CYAN            0b0011
#define RED             0b0100
#define MAGENTA         0b0101
#define BROWN           0b0110
#define LIGHT_GREY      0b0111
#define DARK_GREY       0b1000
#define LIGHT_BLUE      0b1001
#define LIGHT_GREEN     0b1010
#define LIGHT_CYAN      0b1011
#define LIGHT_RED       0b1100
#define LIGHT_MAGENTA   0b1101
#define YELLOW          0b1110
#define WHITE           0b1111


// Current directory variables
extern uint32_t current_directory;
extern struct FAT32DirectoryTable current_dir_table;
extern char current_path[512];

// Run a system call to the kernel
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

// Wrapper function for printing strings to screen
void put(char* str, uint8_t color);

// Parse path string into array of file/folder names
void parse_path(char* str, char paths[][512], uint32_t* num_of_directory);

// Print current directory to screen
void print_current_directory();

// Update directory table to the given cluster number
void update_directory_table(struct FAT32DirectoryTable dir_table, uint32_t directory_cluster_number);

// Print path up to path[idx] to screen
void print_path(char path[][512], uint32_t idx, uint8_t color);

// Appends the current shell path
void append_current_path(char* path);

// Retracts the current shell path to its parent folder
void retract_current_path();

#endif

