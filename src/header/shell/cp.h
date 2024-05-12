#ifndef _CP_H
#define _CP_H

#include "header/shell/user-shell.h"

// User command 'cp'
void cp(char args[][512], int args_count);

// Copies file from file_path to dest_path
void copy_file(char file_path[][512], uint32_t file_num_of_directory, char dest_path[][512], uint32_t dest_num_of_directory);

// Reads file from the specified path to the buffer
int8_t read_file(char path[][512], uint32_t num_of_directory, struct ClusterBuffer buffer[]);

// Parses file name to file and ext
int8_t parse_file_name(char* file_name, char* name, char* ext);

// Finds the actual used buffer size
int32_t find_buffer_size(struct ClusterBuffer buf[]);

#endif