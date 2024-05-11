#ifndef _MKDIR_H
#define _MKDIR_H

#include "header/shell/user-shell.h"

// User command 'mkdir'
void mkdir(char args[][512], int args_count);

// Creates directory at the specified path
void create_directory(char args[][512], uint32_t num_of_directory);

// Returns true if dir name contains the invalid character '.'
bool contains_invalid_character(char* dir_name);

// Writes a file or folder to the specified path
int8_t write_to(char* name, char path[][512], uint32_t num_of_directory, struct ClusterBuffer buf[], uint32_t buffer_size, bool is_directory);

#endif