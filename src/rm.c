#include "header/shell/rm.h"
#include "header/shell/cd.h"
#include "header/shell/cp.h"

void rm(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: rm <directory_path_to_file><ext>\n", LIGHT_GREY);
            break;
        case 1:
            {char path[512][512];
            uint32_t current_directory_temp = current_directory;
            char path_temp[512];
            memcpy(path_temp, current_path, sizeof(path_temp));

            memset(path, 0, 512*512);
            uint32_t num_of_directory = 0;
            parse_path(args[0], path, &num_of_directory);
            
            int8_t ret_val = change_path(path, num_of_directory-1);
            if (ret_val != 0) {
                cancel_change_path(current_directory_temp, path_temp);
                return;
            }

            char file_name[9] = {0}, ext[4] = {0};
            char* name = path[num_of_directory - 1];
            parse_file_name(name, file_name, ext);

            struct FAT32DriverRequest request = {
                .parent_cluster_number  = current_directory,
            };
            memcpy(request.name, file_name, 8);
            memcpy(request.ext, ext, 3);

            syscall(3, (uint32_t) &request, (uint32_t) &ret_val, 0);
            switch (ret_val)
            {
            case 1:
                put("error: '", LIGHT_RED);
                put(file_name, LIGHT_RED);
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
            case 2:
                put("error: ", LIGHT_RED);
                if (num_of_directory >= 2) {
                    put("'", LIGHT_RED);
                    print_path(path, num_of_directory - 2, LIGHT_RED);
                }
                else {
                    put("current directory '", LIGHT_RED);
                    put(current_dir_table.table[0].name, LIGHT_RED);
                }
                put("' is not empty\n", LIGHT_RED);
                break;
            default:
                break;
            }
            cancel_change_path(current_directory_temp, path_temp);
            break;}
            
        default:
            put("error: too many arguments, expected 1 argument\n", LIGHT_RED);
            put("Usage: rm <directory_path_to_file><ext>\n", LIGHT_GREY);
            break;       
    }
}