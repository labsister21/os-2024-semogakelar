#include "header/shell/cat.h"
#include "header/shell/cd.h"
#include "header/shell/cp.h"

void cat(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: cat <directory_path_to_file><ext>\n", LIGHT_GREY);
            break;
        case 1:
            {char path[512][512];
            memset(path, 0, 512*512);
            uint32_t num_of_directory = 0;
            parse_path(args[0], path, &num_of_directory);
            
            struct ClusterBuffer buffer[32];
            int8_t ret_val = read_file(path, num_of_directory, buffer);
            if (ret_val != 0)
                return;

            put((char*) buffer, WHITE);
            put("\n", WHITE);
            break;}
        default:
            put("error: too many arguments, expected 1 argument\n", LIGHT_RED);
            put("Usage: cat <directory_path_to_file><ext>\n", LIGHT_GREY);
            break;       
    }
}