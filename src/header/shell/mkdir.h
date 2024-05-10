#ifndef _MKDIR_H
#define _MKDIR_H

#include "header/shell/user-shell.h"

// User command 'mkdir'
void mkdir(char args[][512], int args_count);

// Creates directory at the specified path
void create_directory(char args[][512], uint32_t num_of_directory);


#endif