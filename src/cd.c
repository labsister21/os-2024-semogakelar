#include "header/shell/user-shell.h"

void change_directory(char path[][512], int num_of_directory) {
    put("yayyy", WHITE);
    path[0][0] = '\0';
    num_of_directory++; num_of_directory--;
}

void cd(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", RED);
            put("Usage: cd <directory_path>\n", WHITE);
            break;
        case 1:
            char path[512][512];
            uint32_t num_of_directory = 0;
            parse_path(args[0], path, &num_of_directory);

            change_directory(path, num_of_directory);
            break;
        default:
            put("error: too many arguments\n", RED);
            put("Usage: cd <directory_path>\n", WHITE);
            break;
    }
     
}