#ifndef _LS_H_
#define _LS_H_

#include "user-shell.h"

// User command 'ls'
void ls(int args_count);

// Print directory entries in current directory
void print_directory_entries();

// Print a directory entry in current directory
void print_directory_entry(int idx, bool spaces);

#endif