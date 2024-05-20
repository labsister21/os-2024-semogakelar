#ifndef _EXEC_H
#define _EXEC_H

#include "header/shell/user-shell.h"

// User command 'exec'
void exec(char args[][512], int args_count);

// Execute program 
void execute_program(char file_path[][512], uint32_t file_num_of_directory);

#endif