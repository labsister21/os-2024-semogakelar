#include "header/shell/exec.h"
#include "header/shell/cd.h"
#include "header/shell/cp.h"

void exec(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: exec <file_directory_path>\n", LIGHT_GREY);
            break;
        case 1:
            char file_path[512][512] = {0};
            uint32_t file_num_of_directory = 0;
            parse_path(args[0], file_path, &file_num_of_directory);
            execute_program(file_path, file_num_of_directory);
            break;
        default:
            put("error: too many arguments, expected 1 argument\n", LIGHT_RED);
            put("Usage: exec <file_directory_path>\n", LIGHT_GREY);
            break;
    }
}

void execute_program(char file_path[][512], uint32_t num_of_directory) {
    int8_t ret_val = change_path(file_path, num_of_directory - 1);
    if (ret_val != 0)
        return;

    char file_name[9] = {0}, ext[4] = {0};
    ret_val = parse_file_name(file_path[num_of_directory - 1], file_name, ext);
    if (ret_val != 0)
        return;

    struct ClusterBuffer buffer[32];
    struct FAT32DriverRequest request = {
        .buf                   = buffer,
        .parent_cluster_number = current_directory,
        .buffer_size           = 0x100000
    };
    memcpy(request.name, file_name, 8);
    memcpy(request.ext, ext, 3);

    syscall(12, (uint32_t) &request, (uint32_t) &ret_val, 0);

    if (ret_val != 0) {
        put("error: ", LIGHT_RED);
        switch (ret_val) {
            case 1:
                put("maximum active processes exceeded, need to terminate another active process first\n", LIGHT_RED);
                break;
            case 2:
                put("invalid entrypoint, access to kernel section is forbidden\n", LIGHT_RED);
                break;
            case 3:
                put("not enough available memory, file is too big\n", LIGHT_RED);
                break;
            case 4:
                put("read executable file failed, make sure there's a file named '\n", LIGHT_RED);
                put(file_name, LIGHT_RED);
                if (strlen(ext) != 0) {
                    put(".", LIGHT_RED);
                    put(ext, LIGHT_RED);
                }
                put("' in ", LIGHT_RED);
                if (num_of_directory > 2) {
                    put("'", LIGHT_RED);
                    print_path(file_path, num_of_directory-2, LIGHT_RED);
                    put("'", LIGHT_RED);
                } else {
                    put("current directory", LIGHT_RED);
                }
                
                put("\n", LIGHT_RED);
                break;
        }
    }
}