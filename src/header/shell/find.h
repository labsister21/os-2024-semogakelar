#ifndef _FIND_H
#define _FIND_H

#include "header/shell/user-shell.h"

// User command 'find'
void find(char args[][512], int args_count);

// Creates directory at the specified path
void trackToParent(int);

#endif