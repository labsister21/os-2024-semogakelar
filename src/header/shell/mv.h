#ifndef _MV_H
#define _MV_H

#include "header/shell/user-shell.h"
#include "header/shell/cp.h"

// User command 'cd'
void mv(char args[][512], int args_count);

void rename(char*, char*);

#endif