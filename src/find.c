#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "header/stdlib/string.h"
#include "header/shell/find.h"
#include "header/filesystem/fat32.h"

void find(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: mkdir <directory_path>\n", WHITE);
            break;
        case 1:
            {char* goal = args[0];
            int Queue[2048];

            int currCluster = 2;
            int idxQueue = 0;
            int finalPathCount = 0;
            Queue[idxQueue] = currCluster;
            while (Queue[idxQueue] != 0) {
                currCluster = Queue[idxQueue];
                struct FAT32DirectoryTable currDirectory = {0};
                syscall(9, (uint32_t) &currDirectory, currCluster, 0);
                int index = 2;
                int tempIdx = idxQueue + 1;
                while (index < 64) {
                    char currName[8] = {0};
                    memcpy(currName, currDirectory.table[index].name, sizeof(currName));
                    if (strlen(currName) == strlen(goal) && memcmp(currName, goal, strlen(goal)) == 0) {
                        trackToParent(currCluster, goal);
                        finalPathCount++;
                    }
                    if (currDirectory.table[index].user_attribute == UATTR_NOT_EMPTY &&
                        currDirectory.table[index].attribute == ATTR_SUBDIRECTORY){
                        Queue[tempIdx] = currDirectory.table[index].cluster_low;
                        tempIdx++;
                    }
                    index++;
                }
                idxQueue++;
            }

            if (finalPathCount == 0) {
                put("error: there are no folder or file with the name '", LIGHT_RED);
                put(goal, LIGHT_RED);
                put("'\n", LIGHT_RED);
            }
            break;}
        default:
            put("error: too many arguments, expected 1 argument\n", LIGHT_RED);
            put("Usage: find <file/directory_name>\n", WHITE);
            break;
    }
}


void trackToParent(int currCluster, char goal[8]) {
    struct FAT32DirectoryTable currDirectory = {0};
    char listPath[512][512] = {0};
    int idxListPath = 0;
    while (currDirectory.table[0].cluster_low != 2) {
        syscall(9, (uint32_t) &currDirectory, currCluster, 0);
        memcpy(listPath[idxListPath], currDirectory.table[0].name, 8);
        currCluster = currDirectory.table[1].cluster_low;
        idxListPath++;
    }
    for (int i = idxListPath; i >= 0; i--) {
        put(listPath[i], WHITE);
        put("/", WHITE);
    }
    put(goal, WHITE);
    put("\n", WHITE);
}