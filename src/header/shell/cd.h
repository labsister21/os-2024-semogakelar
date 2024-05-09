#ifndef _CD_H
#define _CD_H

// User command 'cd'
void cd(char args[][512], int args_count);

// Change current directory to the specified path
void change_directory(char path[][512], int num_of_directory);

#endif