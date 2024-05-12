#include "header/shell/mv.h"
#include "header/shell/cd.h"
#include "header/shell/rm.h"
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

void mv(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: mv <directory_path>\n", WHITE);
            break;
        case 2:
            char dest_path[512][512] = {0};
            uint32_t num_dest_directory = 0;
            char src_path[512][512] = {0};
            uint32_t num_src_directory = 0;
            parse_path(args[0], src_path, &num_src_directory);
            parse_path(args[0], dest_path, &num_dest_directory);

            // if ()
            move(args[0], args[1]);
            // if (contains_invalid_char(args[1], '/') || contains_invalid_char(args[1], '\\')){
            // } else {
            //     rename(args[0], args[1]);
            //     put("success\n", GREEN);
            // }
            break;
        default:
            put("error: too many arguments, expected 2 argument\n", LIGHT_RED);
            put("Usage: mv <directory_path>\n", WHITE);
            break;
    }
}

bool contains_invalid_char(char* dir_name, char invalid_char) {
    for (size_t i = 0; i < strlen(dir_name); i++) {
        if (dir_name[i] == invalid_char) {            
            return true;
        }
    }
    return false;
}

void move(char args0[], char args1[]) {
    uint32_t num_src_directory;
    uint32_t num_dest_directory;
    char srcPath[512][512] = {0};
    char destPath[512][512] = {0};
    parse_path(args0, srcPath, &num_src_directory);
    parse_path(args1, destPath, &num_dest_directory);
    struct FAT32DirectoryTable srcDirectory = {0};
    uint32_t srcCluster = 0;
    struct FAT32DirectoryTable destDirectory = {0};
    uint32_t destCluster = 0;
    struct FAT32DirectoryTable parentSrc = {0};
    uint32_t parentSrcCluster = 0;
    
    bool isfile;
    char temPath[512];
    memcpy(&temPath, &current_path, 512);
    change_path(srcPath, num_src_directory - 1, &parentSrc, &parentSrcCluster);
    memcpy(&current_path, &temPath, 512);
    int idx = 2;
    while (idx < 64) {
        if (memcmp(parentSrc.table[idx].name, srcPath[num_src_directory - 1], 8)) {
            if (parentSrc.table[idx].attribute != ATTR_SUBDIRECTORY) {
                isfile = true;
            } else {
                isfile = false;
            }
            break;
        }
        idx++;
    }
    if (!isfile) {
        change_path(srcPath, num_src_directory, &srcDirectory, &srcCluster);
        memcpy(&current_path, &temPath, 512);
    }
    put("mulai move\n", WHITE);

    memcpy(&srcDirectory, &current_dir_table, CLUSTER_SIZE);
    put("mulai move\n", WHITE);

    int ret = change_path(destPath, num_dest_directory, &destDirectory, &destCluster);
    put("mulai move\n", WHITE);
    memcpy(&current_path, &temPath, 512);
    if (ret != 0){
        return;
    }
    put("mulai move\n", WHITE);
    memcpy(&destDirectory, &current_dir_table, CLUSTER_SIZE);

    int indexTable = 1;
    int count = 0;
    while (indexTable < 64) {
        if (destDirectory.table[indexTable].attribute != UATTR_NOT_EMPTY) {
            count++;
            break;
        }
        indexTable++;
    }
    put("mulai move\n", WHITE);
    if (count == 0){
        put(args1, WHITE);
        put(" is full, mv failed\n", LIGHT_RED);
    } else {
        memset(&parentSrc.table[idx], 0, sizeof(struct FAT32DirectoryEntry));\
        syscall(10, (uint32_t) &parentSrc, parentSrcCluster, 0);
        if (!isfile) {
            memcpy(&srcDirectory.table[1], &destDirectory.table[0], sizeof(srcDirectory.table[1]));
            syscall(10, (uint32_t) &srcDirectory, srcCluster, 0);
        }
        int indexTable = 1;
        while (indexTable < 64) {
            if (destDirectory.table[indexTable].attribute != UATTR_NOT_EMPTY) {
                count++;
                break;
            }
            indexTable++;
        }
        if (!isfile) {
            memcpy(&destDirectory.table[indexTable], &srcDirectory.table[0], sizeof(srcDirectory.table[1]));
        } else {
            memcpy(&destDirectory.table[indexTable], &parentSrc.table[0], sizeof(srcDirectory.table[1]));
        }
        syscall(10, (uint32_t) &destDirectory, destCluster, 0);
    }
    put("mulai move\n", WHITE);
}

void rename(char path[], char newName[]) {
    uint32_t num_of_directory;
    char listPath[512][512] = {0};
    parse_path(path, listPath, &num_of_directory);
    int clusterInitial = current_directory;
    char temPath[512];
    memcpy(&temPath, &current_path, 512);
    // change_path(listPath, num_of_directory - 1);
    int indexTable = 0;
    bool found = false;
    while (indexTable < 64) {
        if (memcmp(current_dir_table.table[indexTable].name, path, 8) == 0) {
            found = true;
            break;
        }
        indexTable++;
    }
    if (!found) {
        put("Error: no file/directory ", LIGHT_RED);
        put(listPath[num_of_directory - 1], WHITE);
        put("\n", LIGHT_RED);
    } else {
        memcpy(current_dir_table.table[indexTable].name, newName, 8);
        syscall(10, (uint32_t) &current_dir_table, current_directory, 0);
        if (current_dir_table.table[indexTable].attribute == ATTR_SUBDIRECTORY) {
            int cluster = current_dir_table.table[indexTable].cluster_low;
            syscall(9, (uint32_t) &current_dir_table, cluster, 0);
            memcpy(current_dir_table.table[0].name, newName, 8);
            syscall(10, (uint32_t) &current_dir_table, cluster, 0);   
        }
    }
    cancel_change_path(clusterInitial, temPath);
}