#ifndef _CD_H
#define _CD_H

#include "header/shell/user-shell.h"

// User command 'cd'
void cd(char args[][512], int args_count);

// Change current directory to the specified path
int8_t change_path(char path[][512], int num_of_directory);

// Reverts path to current_directory_temp and path_temp
void cancel_change_path(uint32_t current_directory_temp, char* path_temp);

// Change current directory to a specific directory. Returns error_val if error happens
int8_t change_directory(char* directory_name, uint32_t* parent_cluster_number);

#endif