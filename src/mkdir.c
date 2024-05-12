#include "header/shell/mkdir.h"
#include "header/shell/cp.h"
#include "header/shell/cd.h"

void mkdir(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: mkdir <directory_path>\n", WHITE);
            break;
        case 1:
            {char path[512][512];
            memset(path, 0, 512*512);
            uint32_t num_of_directory = 0;
            parse_path(args[0], path, &num_of_directory);
            create_directory(path, num_of_directory);
            break;}
        default:
            put("error: too many arguments, expected 1 argument\n", LIGHT_RED);
            put("Usage: mkdir <directory_path>\n", WHITE);
            break;
    }
}

void create_directory(char path[][512], uint32_t num_of_directory) {
    if (!contains_invalid_character(path[num_of_directory - 1])) {
        write_to(path[num_of_directory - 1], path, num_of_directory, (struct ClusterBuffer*) 0x0, 0, true);
    }
}

int8_t write_to(char* name, char path[][512], uint32_t num_of_directory, struct ClusterBuffer buf[], uint32_t buffer_size, bool is_directory) {
    uint32_t current_directory_temp = current_directory;
    char path_temp[512];
    memcpy(path_temp, current_path, sizeof(path_temp));

    int8_t ret_val = change_path(path, (is_directory)? num_of_directory-1 : num_of_directory);
    if (ret_val != 0) {
        cancel_change_path(current_directory_temp, path_temp);
        return ret_val;
    }

    char file_name[9] = {0}, ext[4] = {0};
    if (!is_directory) {
        parse_file_name(name, file_name, ext);
    }

    struct FAT32DriverRequest request = {
        .buf                    = buf,
        .parent_cluster_number  = current_directory,
        .buffer_size            = buffer_size
    };
    if (is_directory) {
        memcpy(request.name, name, sizeof(request.name));
        memcpy(request.ext, "\0\0\0", 3);
    } else {
        memcpy(request.name, file_name, 8);
        memcpy(request.ext, ext, 3);
    }
    
    syscall(2, (uint32_t) &request, (uint32_t) &ret_val, 0);

    switch (ret_val) {
        case -1:
            put("error: cannot add new entry, ", LIGHT_RED);
            if (num_of_directory >= 2) {
                put("'", LIGHT_RED);
                print_path(path, num_of_directory - 2, LIGHT_RED);
            } else {
                put("current directory '", LIGHT_RED);
                put(current_dir_table.table[0].name, LIGHT_RED);
            }
            put("' is full\n", LIGHT_RED);
            break;
        case 1:
            put("error: there's already a ", LIGHT_RED);
            put(is_directory? "folder" : "file", LIGHT_RED);
            put(" with the name '", LIGHT_RED);
            put(name, LIGHT_RED);
            put("' in ", LIGHT_RED);
            if (num_of_directory > 1) {
                put("'", LIGHT_RED);
                print_path(path, num_of_directory - 2, LIGHT_RED);
            } else {
                put("current directory '", LIGHT_RED);
                put(current_dir_table.table[0].name, LIGHT_RED);
            }
            put("'\n", LIGHT_RED);
            break;
    }

    cancel_change_path(current_directory_temp, path_temp);
    return ret_val;
}

bool contains_invalid_character(char* dir_name) {
    for (size_t i = 0; i < strlen(dir_name); i++) {
        if (dir_name[i] == '.') {
            put("error: name of the new directory '", LIGHT_RED);
            put(dir_name, LIGHT_RED);
            put("' cannot contain invalid character '.'\n", LIGHT_RED);
            
            return true;
        }
    }
    return false;
}