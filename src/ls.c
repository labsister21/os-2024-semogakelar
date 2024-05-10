#include "header/shell/ls.h"

void ls(int args_count) {
    switch (args_count) {
        case 0:
            print_directory_entries();
            break;
        default:
            put("error: too many arguments, expected 0 argument\n", LIGHT_RED);
            put("Usage: ls\n", LIGHT_GREY);
            break;
    }
}

void print_directory_entries() {
    uint8_t idx = 2;
    bool found = false;

    // Print first directory entry
    while (idx < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) && 
           current_dir_table.table[idx].user_attribute != UATTR_NOT_EMPTY) {
        idx++;
    }
    if (idx < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) {
        print_directory_entry(idx, false);
        found = true;
        idx++;
    }

    // Print the rest of directory entries
    while (idx < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) {
        if (current_dir_table.table[idx].user_attribute == UATTR_NOT_EMPTY) {
            print_directory_entry(idx, true);
            found = true;
        }
        idx++;
    }
    
    if (!found) {
        put("Current directory '", LIGHT_GREY);
        put(current_dir_table.table[0].name, LIGHT_GREY);
        put("' is empty", LIGHT_GREY);
    }

    put("\n", WHITE);
}

void print_directory_entry(int idx, bool spaces) {
    if (spaces) put("    ", WHITE);
    if (current_dir_table.table[idx].attribute == ATTR_SUBDIRECTORY) {
        put(current_dir_table.table[idx].name, LIGHT_BLUE);
    }
    else {
        put(current_dir_table.table[idx].name, WHITE);
        if (memcmp(current_dir_table.table[idx].ext, "\0\0\0", 3) != 0) {
            put(".", WHITE);
            put(current_dir_table.table[idx].ext, WHITE);
        }
    }
}