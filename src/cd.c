#include "header/shell/cd.h"

struct FAT32DirectoryTable temp_dir_table;

int8_t change_path(char path[][512], int num_of_directory) {
    if (num_of_directory == 0) {
        return 0;
    }
    
    int32_t idx = 0;
    uint32_t temp_parent_cluster_number = current_directory;
    memcpy(&temp_dir_table, &current_dir_table, sizeof(struct FAT32DirectoryTable));

    int8_t err_val = 0;
    char path_temp[512][512] = {0};
    memcpy(path_temp, current_path, 512*512);

    while (err_val == 0 && idx < num_of_directory) {
        if (strlen(path[idx]) == 2 && memcmp(path[idx], "..", 2) == 0) {
            if (strlen(current_path) == 1 && memcmp(current_path, "/", 1) == 0) {
                continue;
            }
            temp_parent_cluster_number = temp_dir_table.table[1].cluster_low | temp_dir_table.table[1].cluster_high << 16;
            update_directory_table(&temp_dir_table, temp_parent_cluster_number);
            retract_current_path();
        }
        else if (!(strlen(path[idx]) == 1 && memcmp(path[idx], ".", 1) == 0)) {
            err_val = change_directory(path[idx], &temp_parent_cluster_number);
            switch (err_val) {
                case 1:
                    put("error: '", LIGHT_RED);
                    print_path(path, idx, LIGHT_RED);
                    put("' is a file, not a directory\n", LIGHT_RED);
                    break;
                case 2:
                    put("error: no such directory with the name '", LIGHT_RED);
                    put(path[idx], LIGHT_RED);
                    put("' in ", LIGHT_RED);
                    if (idx > 0) {
                        put("'", LIGHT_RED);
                        print_path(path, idx - 1, LIGHT_RED);
                        put("'", LIGHT_RED);
                    } else {
                        put("current directory", LIGHT_RED);
                    }
                    put("\n", LIGHT_RED);
                    break;
            }
        }
        idx++;
    }

    if (err_val == 0) {
        memcpy(&current_dir_table, &temp_dir_table, sizeof(struct FAT32DirectoryTable));
        current_directory = temp_parent_cluster_number;
    } else {
        memcpy(current_path, path_temp, 512*512);
    }

    return err_val;
}

void cancel_change_path(uint32_t current_directory_temp, char* path_temp) {
    current_directory = current_directory_temp;
    update_directory_table(&current_dir_table, current_directory);
    memcpy(current_path, path_temp, sizeof(current_path));
}

int8_t change_directory(char* directory_name, uint32_t* parent_cluster_number) {
    struct FAT32DriverRequest request = {
        .buf                    = &temp_dir_table,
        .ext                    = "\0\0\0",
        .parent_cluster_number  = *parent_cluster_number,
        .buffer_size            = sizeof(temp_dir_table)
    };
    int8_t ret_val;
    memcpy(request.name, directory_name, 8);
    syscall(1, (uint32_t) &request, (uint32_t) &ret_val, 0);

    if (ret_val != 0)
        return ret_val;

    uint32_t cluster_number = temp_dir_table.table[0].cluster_low | temp_dir_table.table[0].cluster_high << 16;
    *parent_cluster_number = cluster_number;
    append_current_path(temp_dir_table.table[0].name);

    return 0;
}

void cd(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: cd <directory_path>\n", LIGHT_GREY);
            break;
        case 1:
            {char path[512][512];
            memset(path, 0, 512*512);
            uint32_t num_of_directory = 0;
            parse_path(args[0], path, &num_of_directory);
            change_path(path, num_of_directory);
            break;}
        default:
            put("error: too many arguments, expected 1 argument\n", LIGHT_RED);
            put("Usage: cd <directory_path>\n", LIGHT_GREY);
            break;
    }
}