#ifndef _CAT_H_
#define _CAT_H_
#include "user-shell.h"

// User command 'cat'
void cat(char args[][512], int args_count);

// Display file from filepath to screen
void display_file(char path[][512], uint32_t num_of_directory);

#endif