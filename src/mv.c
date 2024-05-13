#include "header/shell/mv.h"
#include "header/shell/cd.h"
#include "header/shell/rm.h"
#include "header/shell/cp.h"
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

void mv(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: mv <directory_path>\n", WHITE);
            break;
        case 2:
            {move(args[0], args[1]);
            break;}
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
    uint32_t initialCluster = 0;

    char name[8] = {0}, ext[3] = {0};
    parse_file_name(srcPath[num_src_directory - 1], name, ext);

    bool isfile;
    char temPath[512];
    memcpy(temPath, current_path, 512);
    initialCluster = current_directory;

    if (num_src_directory == 1) {
        memcpy(&parentSrc, &current_dir_table, sizeof(struct FAT32DirectoryTable));
        parentSrcCluster = current_directory;
    } else {
        int ret_val = change_path(srcPath, num_src_directory - 1);
        if (ret_val != 0) {
            cancel_change_path(initialCluster, temPath);
            return;
        }
        memcpy(&parentSrc, &current_dir_table, sizeof(struct FAT32DirectoryTable));
        parentSrcCluster = current_directory;
        cancel_change_path(initialCluster, temPath);
    }
    int idx = 2;
    while (idx < 64) {
        if (memcmp(parentSrc.table[idx].name, name, 8) == 0 &&
            memcmp(parentSrc.table[idx].ext, ext, 3) == 0) {
            if (parentSrc.table[idx].attribute != ATTR_SUBDIRECTORY) {
                isfile = true;
            } else {
                isfile = false;
            }
            break;
        }
        idx++;
    }
    if (idx == 64) {
        put("error: there are no folder or file with the name '", LIGHT_RED);
        put(srcPath[num_src_directory - 1], LIGHT_RED);
        put("' in '", LIGHT_RED);
        print_path(srcPath, num_src_directory - 2, LIGHT_RED);
        put("'\n", LIGHT_RED);
    }

    if (!isfile) {
        change_path(srcPath, num_src_directory);
        memcpy(&srcDirectory, &current_dir_table, sizeof(struct FAT32DirectoryTable));
        srcCluster = current_directory;
        cancel_change_path(initialCluster, temPath);
    }

    int ret = change_path(destPath, num_dest_directory);
    memcpy(&destDirectory, &current_dir_table, sizeof(struct FAT32DirectoryTable));
    destCluster = current_directory;
    cancel_change_path(initialCluster, temPath);
    memcpy(current_path, temPath, 512);
    if (ret != 0){
        return;
    }

    int indexTable = 2;
    while (indexTable < 64) {
        if (destDirectory.table[indexTable].attribute != UATTR_NOT_EMPTY) {
            break;
        }
        indexTable++;
    }

    if (indexTable == 64){
        put("error: '", LIGHT_RED);
        print_path(destPath, num_dest_directory - 1, LIGHT_RED);
        put("' is full, mv failed\n", LIGHT_RED);
    } else {
        if (!isfile) {
            memcpy(&srcDirectory.table[1], &destDirectory.table[0], sizeof(struct FAT32DirectoryEntry));
            syscall(10, (uint32_t) &srcDirectory, srcCluster, 0);
        }
        memcpy(&destDirectory.table[indexTable], &parentSrc.table[idx], sizeof(struct FAT32DirectoryEntry));
        memset(&parentSrc.table[idx], 0, sizeof(struct FAT32DirectoryEntry));

        syscall(10, (uint32_t) &parentSrc, parentSrcCluster, 0);
        syscall(10, (uint32_t) &destDirectory, destCluster, 0);
    }
    update_directory_table(&current_dir_table, current_directory);
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