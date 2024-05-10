#include "header/shell/cat.h"
#include "header/shell/cd.h"

void cat(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: cat <directory_path_to_file><ext>\n", LIGHT_GREY);
            break;
        case 1:
            char path[512][512];
            memset(path, 0, 512*512);
            uint32_t num_of_directory = 0;
            parse_path(args[0], path, &num_of_directory);
            display_file(path, num_of_directory);
            break;
        default:
            put("error: too many arguments, expected 1 argument\n", LIGHT_RED);
            put("Usage: cat <directory_path_to_file><ext>\n", LIGHT_GREY);
            break;       
    }
}

void display_file(char path[][512], uint32_t num_of_directory) {
    uint32_t current_directory_temp = current_directory;
    char path_temp[512];
    memcpy(path_temp, current_path, sizeof(path_temp));

    int8_t ret_val = change_path(path, num_of_directory - 1);
    if (ret_val != 0)
        return;

    //////////////////////////////////////////////////////////////////
    int flag = 0;
    int i = 0;
    while (flag == 0 || i < 64) {
        if (strlen(current_dir_table.table[i].name) == strlen(path[num_of_directory - 1]) &&
            memcmp(current_dir_table.table[i].name, path[num_of_directory - 1], sizeof(current_dir_table.table[i].name)) == 0) {
            
            flag = 1;
        }
        i++;
    }
    i--;

    if (flag == 0) {
        put("error: no such file or directory\n", LIGHT_RED);
        return;
    }
    if (current_dir_table.table[i].attribute != ATTR_SUBDIRECTORY) {
        put("error: '", LIGHT_RED);
        put(path[num_of_directory - 1], LIGHT_RED);
        put("' is a directory\n", LIGHT_RED);
        return;
    }

    struct ClusterBuffer c[CLUSTER_SIZE*64] = {0};
    struct FAT32DriverRequest request = {
        .buf                    = &c,
        .parent_cluster_number  = current_directory,
        .buffer_size            = sizeof(c)
    };
    memcpy(request.name, current_dir_table.table[i].name, 8);
    memcpy(request.ext, current_dir_table.table[i].ext, 8);

    syscall(0, (uint32_t) &request, (uint32_t) &ret_val, 0);

    // if ret_val != 0, tampilin pesan error sesuai ret_val

    put((char*) c, WHITE);
    //////////////////////////////////////////////////////////////////

    current_directory = current_directory_temp;
    update_directory_table(&current_dir_table, current_directory);
    memcpy(current_path, path_temp, sizeof(current_path));
}