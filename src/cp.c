#include "header/shell/cp.h"
#include "header/shell/cd.h"
#include "header/shell/mkdir.h"

void cp(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: cp <source_file_directory_path> <dest_directory_path>\n", LIGHT_GREY);
            break;
        case 1:
            put("error: too few arguments, expected 2 argument\n", LIGHT_RED);
            put("Usage: cp <source_file_directory_path> <dest_directory_path>\n", LIGHT_GREY);
            break;
        default:
            char file_path[512][512] = {0}, dest_path[512][512] = {0};
            uint32_t file_num_of_directory = 0, dest_num_of_directory = 0;
            parse_path(args[0], file_path, &file_num_of_directory);
            parse_path(args[1], dest_path, &dest_num_of_directory);
            
            copy_file(file_path, file_num_of_directory, dest_path, dest_num_of_directory);
            break;
    }
}

void copy_file(char file_path[][512], uint32_t file_num_of_directory, char dest_path[][512], uint32_t dest_num_of_directory) {
    struct ClusterBuffer buffer[32];
    int8_t ret_val = read_file(file_path, file_num_of_directory, buffer);
    if (ret_val != 0)
        return;

    write_to(file_path[file_num_of_directory - 1], dest_path, dest_num_of_directory, buffer, find_buffer_size(buffer), false);
}

int8_t read_file(char path[][512], uint32_t num_of_directory, struct ClusterBuffer buffer[]) {
    uint32_t current_directory_temp = current_directory;
    char path_temp[512];
    memcpy(path_temp, current_path, sizeof(path_temp));

    int8_t ret_val = change_path(path, num_of_directory - 1 , &current_dir_table, &current_directory);
    if (ret_val != 0) {
        cancel_change_path(current_directory_temp, path_temp);
        return ret_val;
    }

    char file_name[9] = {0}, ext[4] = {0};
    ret_val = parse_file_name(path[num_of_directory - 1], file_name, ext);
    if (ret_val != 0) {
        cancel_change_path(current_directory_temp, path_temp);
        return ret_val;
    }

    struct FAT32DriverRequest request = {
        .buf                    = buffer,
        .parent_cluster_number  = current_directory,
        .buffer_size            = CLUSTER_SIZE * 32
    };
    memcpy(request.name, file_name, 8);
    memcpy(request.ext, ext, 3);

    syscall(0, (uint32_t) &request, (uint32_t) &ret_val, 0);

    if (ret_val != 0) {
        put("error: '", LIGHT_RED);
        put(file_name, LIGHT_RED);
        switch (ret_val) {
            case 1:
                put("' is a folder, not a file\n", LIGHT_RED);
                break;
            case 2:
                put("' not found in ", LIGHT_RED);
                if (num_of_directory >= 2) {
                    put("'", LIGHT_RED);
                    print_path(path, num_of_directory - 2, LIGHT_RED);
                }
                else {
                    put("current directory '", LIGHT_RED);
                    put(current_dir_table.table[0].name, LIGHT_RED);
                }
                put("'\n", LIGHT_RED);
                break;
        }
    }

    cancel_change_path(current_directory_temp, path_temp);
    return ret_val;
}

int8_t parse_file_name(char* file_name, char* name, char* ext) {
    size_t idx1 = 0, idx2 = 0;
    while (idx1 < strlen(file_name) && file_name[idx1] != '.' && idx2 < 8) {
        name[idx2] = file_name[idx1];
        idx1++; idx2++;
    }

    if (idx2 == 8 && idx1 < strlen(file_name) && file_name[idx1] != '.') {
        put("error: filename length must not exceed 8!", LIGHT_RED);
        return 1;
    }

    idx1++; idx2 = 0;
    while (idx1 < strlen(file_name) && idx2 < 3) {
        ext[idx2] = file_name[idx1];
        idx1++; idx2++;
    }

    if (idx1 < strlen(file_name)) {
        put("error: file ext length must not exceed 3!", LIGHT_RED);
        return -1;
    }

    return 0;
}

int32_t find_buffer_size(struct ClusterBuffer buf[]) {
    uint32_t idx = 0;
    bool empty = false;
    while (idx < 64) {
        if (buf[idx].buf[0] == '\0') {    
            empty = true;
            for (int i = 0; i < CLUSTER_SIZE; i++) {
                if (buf[idx].buf[i] != '\0') {
                    empty = false;
                    break;
                }
            }
        }
        if (empty) break;
        idx++;
    }
    return idx * CLUSTER_SIZE;
}