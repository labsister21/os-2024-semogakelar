#include "cat.h"

void cat(FAT32DirectoryTable dir_table, char* file_name) {
    if (strlen(file_name) > 8) {
        put("No such file or directory\n", WHITE);
        return;
    }

    int flag = 0;
    int i = 0;
    while (flag == 0 || i < 64) {
        if (memcmp(dir_table.table[i].name, file_name, sizeof(dir_table.table[i].name)) == 0) {
            flag = 1;
        }
        i++;
    }
    i--;
    if (flag == 0) {
        put("No such file or directory\n", WHITE);
        return;
    }
    if (dir_table.table[i].attribute != ATTR_SUBDIRECTORY) {
        put(file_name, WHITE);
        put(" is a directory\n", WHITE);
    }

    struct ClusterBuffer c[CLUSTER_SIZE*64] = {0};
    struct FAT32DriverRequest temp = {
        .buf                    = &c;
        .name                   = dir_table.table[i].name;
        .ext                    = dir_table.table[i].ext;
        .parent_cluster_number  = dir_table;
        .buffer_size            = sizeof(c);
    }

    int8_t read_file = read(temp);
    j = 0;
    while (c[j] != '\0') {
        putchar(c[j],WHITE);
        j++;
    }
}