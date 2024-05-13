#ifndef _MV_H
#define _MV_H

#include "header/shell/user-shell.h"

// User command 'mv'
void mv(char args[][512], int args_count);

void rename(char path[], char newName[]);

bool contains_invalid_char(char* dir_name, char invalid_char);

void move(char args0[], char args1[]);

#endif