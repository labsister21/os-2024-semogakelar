#include "header/shell/ls.h"

void ls(FAT32DirectoryTable dir_table) {
    int i = 0;
    while (dir_table.table[i] != FAT32_FAT_END_OF_FILE) {
        if (dir_table.table[i].attribute == ATTR_SUBDIRECTORY) {
            put(dir_table.table[i].name, LIGHT_BLUE);
            put("    ", WHITE);
        } else {
            put(dir_table.table[i].name, WHITE);
            putchar('.',WHITE);
            put(dir_table.table[i].ext, WHITE);
            put("    ", WHITE);
        }
        i++;
    }
    putchar('\n',WHITE);
}