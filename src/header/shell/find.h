#ifndef _FIND_H
#define _FIND_H

#include "header/shell/user-shell.h"

// User command 'find'
void find(char args[][512], int args_count);

// Tracks to root folder and prints found entries to screen
void trackToParent(int currCluster, char goal[8]);

#endif