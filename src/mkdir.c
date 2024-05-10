#include "header/shell/mkdir.h"
#include "header/shell/cd.h"

void mkdir(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: mkdir <directory_path>\n", WHITE);
            break;
        case 1:
            char path[512][512];
            memset(path, 0, 512*512);
            uint32_t num_of_directory = 0;
            parse_path(args[0], path, &num_of_directory);
            create_directory(path, num_of_directory);
            break;
        default:
            put("error: too many arguments, expected 1 argument\n", LIGHT_RED);
            put("Usage: mkdir <directory_path>\n", WHITE);
            break;
    }
}

bool contains_invalid_character(char* dir_name) {
    for (size_t i = 0; i < strlen(dir_name); i++) {
        if (dir_name[i] == '.')
            return true;
    }
    return false;
}

void create_directory(char path[][512], uint32_t num_of_directory) {
    if (contains_invalid_character(path[num_of_directory - 1])) {
        put("error: name of the new directory '", LIGHT_RED);
        put(path[num_of_directory - 1], LIGHT_RED);
        put("' cannot contain invalid character '.'\n", LIGHT_RED);
        return;
    }

    uint32_t current_directory_temp = current_directory;
    char path_temp[512];
    memcpy(path_temp, current_path, sizeof(path_temp));

    int8_t ret_val = change_path(path, num_of_directory - 1);
    if (ret_val != 0)
        return;

    struct FAT32DriverRequest request = {
        .buf                    = 0x0,
        .ext                    = "\0\0\0",
        .parent_cluster_number  = current_directory,
        .buffer_size            = 0
    };
    memcpy(request.name, path[num_of_directory - 1], sizeof(request.name));
    syscall(2, (uint32_t) &request, (uint32_t) &ret_val, 0);

    switch (ret_val) {
        case -1:
            put("error: cannot add new directory, ", LIGHT_RED);
            if (num_of_directory >= 2) {
                print_path(path, num_of_directory - 2, LIGHT_RED);
            } else {
                put("current directory", LIGHT_RED);
            }
            put(" is full\n", LIGHT_RED);
            break;
        case 1:
            put("error: there's already an entry with the name '", LIGHT_RED);
            put(path[num_of_directory - 1], LIGHT_RED);
            put("'", LIGHT_RED);
            if (num_of_directory > 1) {
                put("error: in ", LIGHT_RED);
                print_path(path, num_of_directory - 2, LIGHT_RED);
            }
            put("\n", LIGHT_RED);
            break;
        default:
            break;
    }

    current_directory = current_directory_temp;
    update_directory_table(current_dir_table, current_directory);
    memcpy(current_path, path_temp, sizeof(current_path));
}