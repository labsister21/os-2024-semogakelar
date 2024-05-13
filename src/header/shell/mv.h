#ifndef _MV_H
#define _MV_H

#include "header/shell/user-shell.h"
#include "header/shell/cp.h"

// User command 'cd'
void mv(char args[][512], int args_count);

void rename(char*, char*);

bool contains_invalid_char(char[],char);

int8_t change_path_mv(char path[][512], int num_of_directory, struct FAT32DirectoryTable*, uint32_t*);

void move(char args0[], char args1[]);

#endif