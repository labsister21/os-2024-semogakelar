#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

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
extern struct FAT32DirectoryTable dir_table;

// Run a system call to the kernel
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);