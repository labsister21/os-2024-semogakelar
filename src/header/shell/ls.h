#ifndef _LS_H
#define _LS_H

#include "header/shell/user-shell.h"

// User command 'ls'
void ls(int args_count);

// Prints all entries in current directory
void print_directory_entries();

// Prints an entry in current directory
void print_directory_entry(int idx, bool spaces);

#endif