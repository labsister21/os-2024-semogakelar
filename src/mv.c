#include "header/shell/mv.h"
#include "header/shell/cd.h"
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"

void mv(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: mv <directory_path>\n", WHITE);
            break;
        case 2:
            rename(args[0], args[1]);
            put("success\n", GREEN);
            break;
        default:
            put("error: too many arguments, expected 2 argument\n", LIGHT_RED);
            put("Usage: mv <directory_path>\n", WHITE);
            break;
    }
}

// void rename(char path[], char newName[]) {
//     uint32_t num_of_directory;
//     char listPath[512][512] = {0};
//     parse_path(path, listPath, &num_of_directory);
//     int startCluster;
//     if (strlen(path) == 4 && memcmp(path, "root", 4) == 0){
//         startCluster = 2;
//     } else {
//         startCluster = current_directory;
//     }
//     uint32_t index = 1;
//     int currCluster = startCluster;
//     int error = 0;
//     char namefileError[8] = {0};
//     char extfileError[3] = {0};
//     struct FAT32DirectoryTable currDirectory;
//     while (index < num_of_directory - 1 && error == 0) {
//         syscall(9, (uint32_t) &currDirectory, currCluster, 0);
//         if (memcmp(currDirectory.table[0].name, listPath[index], strlen(listPath[index])) == 1) {
//             error = 1;
//             memcpy(&namefileError, currDirectory.table[0].name, 8);
//             memcpy(&extfileError, currDirectory.table[0].ext,3);
//             break;
//         }
//         int indexTable = 2;
//         index++;
//         if (index == num_of_directory) {
//             break;
//         }
//         bool found = false;
//         while (currDirectory.table[indexTable].user_attribute != UATTR_NOT_EMPTY) {
//             if (memcmp(currDirectory.table[indexTable].name, listPath[index], strlen(listPath[index])) == 0) {
//                 currCluster = currDirectory.table[indexTable].cluster_low;
//                 memcpy(&namefileError, currDirectory.table[indexTable].name, 8);
//                 memcpy(&extfileError, currDirectory.table[indexTable].ext,3);
//                 found = true;
//                 break;
//             }
//             indexTable++;
//         }
//         if (!found) {
//             error = 1;
//         }
//     }

//     if (error) {
//         put("Error : There is no file/drectory ", LIGHT_RED);
//         put(namefileError, WHITE);
//         put(".", WHITE);
//         put(extfileError, WHITE);
//     } else {
//         char filename[8];
//         char fileext[3];
//         int err = parse_file_name(newName, filename, fileext);
//         put(filename, WHITE);
//         put("\n",WHITE);
//         put(currDirectory.table[0].name, WHITE);
//         if (!err) {
//             memcpy(currDirectory.table[0].name, filename, 8);
//             memcpy(currDirectory.table[0].ext, fileext, 3);
//             syscall(10, (uint32_t) &currDirectory, currCluster, 0);
//         }
//     }
// }

void rename(char path[], char newName[]) {
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