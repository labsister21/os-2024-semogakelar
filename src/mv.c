#include "header/shell/mv.h"
#include "header/shell/cd.h"
#include "header/shell/rm.h"
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

void mv(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument, expected 2 arguments\n", LIGHT_RED);
            put("Usage: mv <src_directory_path> <dest_directory_path>\n", WHITE);
            break;
        case 1:
            put("error: too few arguments, expected 2 arguments\n", LIGHT_RED);
            put("Usage: mv <src_directory_path> <dest_directory_path>\n", WHITE);
            break;
        case 2:
            {
            char temPath[512];
            memcpy(&temPath, &current_path, 512);
            uint32_t initialCluster = current_directory;

            uint32_t num_dest_directory;
            struct FAT32DirectoryTable tempDir = {0};
            char destPath[512][512] = {0};
            parse_path(args[1], destPath, &num_dest_directory);
            int ret = change_path_mv(destPath, num_dest_directory);
            memcpy(&tempDir, &current_dir_table, sizeof(struct FAT32DirectoryTable));
            cancel_change_path(initialCluster, temPath);
            if (ret == 2){
                if (current_dir_table.table[0].cluster_low == initialCluster) {
                    rename(args[0], destPath[num_dest_directory - 1]);
                } else {
                    put("error: no such directory with the name \n", LIGHT_RED);
                }
            } else {
                move(args[0], args[1]);
            }
            break;
            }
        default:
            put("error: too many arguments, expected 2 arguments\n", LIGHT_RED);
            put("Usage: mv <src_directory_path> <dest_directory_path>\n", WHITE);
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
    parse_path(args1, destPath, &num_dest_directory);
    parse_path(args0, srcPath, &num_src_directory);
    struct FAT32DirectoryTable srcDirectory = {0};
    uint32_t srcCluster = 0;
    struct FAT32DirectoryTable destDirectory = {0};
    uint32_t destCluster = 0;
    struct FAT32DirectoryTable parentSrc = {0};
    uint32_t parentSrcCluster = 0;
    
    uint32_t initialCluster = 0;
    
    bool isfile;
    char temPath[512];
    memcpy(&temPath, &current_path, 512);
    initialCluster = current_directory;

    if (num_src_directory == 1) {
        memcpy(&parentSrc, &current_dir_table, sizeof(struct FAT32DirectoryTable));
        parentSrcCluster = current_directory;
    } else {
        change_path(srcPath, num_src_directory - 1);
        memcpy(&parentSrc, &current_dir_table, sizeof(struct FAT32DirectoryTable));
        put(parentSrc.table[0].name, WHITE);
        put("\n", WHITE);
        parentSrcCluster = current_directory;
        cancel_change_path(initialCluster, temPath);
    }
    int idx = 2;
    while (idx < 64) {
        if (memcmp(parentSrc.table[idx].name, srcPath[num_src_directory - 1], 8) == 0) {
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
        change_path(srcPath, num_src_directory);
        memcpy(&srcDirectory, &current_dir_table, sizeof(struct FAT32DirectoryTable));
        srcCluster = current_directory;
        cancel_change_path(initialCluster, temPath);
    }

    int ret = change_path(destPath, num_dest_directory);
    memcpy(&destDirectory, &current_dir_table, sizeof(struct FAT32DirectoryTable));
    destCluster = current_directory;
    cancel_change_path(initialCluster, temPath);
    memcpy(&current_path, &temPath, 512);
    if (ret != 0){
        return;
    }

    int indexTable = 1;
    int count = 0;
    while (indexTable < 64) {
        if (destDirectory.table[indexTable].attribute != UATTR_NOT_EMPTY) {
            count++;
            break;
        }
        indexTable++;
    }

    if (count == 0){
        put(args1, WHITE);
        put(" is full, mv failed\n", LIGHT_RED);
    } else {
        if (!isfile) {
            memcpy(&srcDirectory.table[1], &destDirectory.table[0], sizeof(srcDirectory.table[1]));
            syscall(10, (uint32_t) &srcDirectory, srcCluster, 0);
        }
        int indexTable = 2;
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
            memcpy(&destDirectory.table[indexTable], &parentSrc.table[idx], sizeof(srcDirectory.table[1]));
        }

        memset(&parentSrc.table[idx], 0, sizeof(struct FAT32DirectoryEntry));
        syscall(10, (uint32_t) &parentSrc, parentSrcCluster, 0);
        syscall(10, (uint32_t) &destDirectory, destCluster, 0);
    }
    update_directory_table(&current_dir_table, current_directory);
}

void rename(char path[], char newName[]) {
    if (contains_invalid_char(newName, '.')) {
        put("error: new filename must not contain '.'\n", LIGHT_RED);
        return;
    }
    uint32_t num_of_directory;
    char listPath[512][512] = {0};
    parse_path(path, listPath, &num_of_directory);
    int clusterInitial = current_directory;
    char temPath[512];
    memcpy(&temPath, &current_path, 512);
    change_path(listPath, num_of_directory - 1);
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

int8_t change_path_mv(char path[][512], int num_of_directory) {
    struct FAT32DirectoryTable temp_dir_table;
    if (num_of_directory == 0) {
        return 0;
    }
    
    int32_t idx = 0;
    uint32_t temp_parent_cluster_number = current_directory;
    memcpy(&temp_dir_table, &current_dir_table, sizeof(struct FAT32DirectoryTable));

    int8_t err_val = 0;
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
                    break;
                case 2:
                    break;
            }
        }
        idx++;
    }

    if (err_val == 0) {
        memcpy(&current_dir_table, &temp_dir_table, sizeof(struct FAT32DirectoryTable));
        current_directory = temp_parent_cluster_number;
    }

    return err_val;
}