#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "header/stdlib/string.h"
#include "header/shell/find.h"
#include "header/filesystem/fat32.h"
#include "dynamicArray.c"

void find(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: mkdir <directory_path>\n", WHITE);
            break;
        case 1:
            char* goal = args[0];
            int Queue[2048];

            int currCluster = 2;
            int idxQueue = 0;
            int finalPathCount = 0;
            struct FAT32DirectoryTable currDirectory;
            Queue[idxQueue] = currCluster;
            while (Queue[idxQueue] != 0) {
                currCluster = Queue[idxQueue];
                syscall(9, (uint32_t) &currDirectory, currCluster, 0);
                int index = 2;
                int tempIdx = idxQueue + 1;
                // int pathSize = size(QueuePath[idxQueue]);
                while (index < 64) {
                    char currName[8];
                    memcpy(currName, currDirectory.table[index].name, strlen(currName));
                    // put(currName, WHITE);
                    // put("\n", WHITE);
                    if (strlen(currName) == strlen(goal) && memcmp(currName, goal, strlen(goal)) == 0) {
                        trackToParent(currCluster);
                        finalPathCount++;
                    }
                    index++;
                    if (currDirectory.table[index].user_attribute != UATTR_NOT_EMPTY){
                        Queue[tempIdx] = currDirectory.table[index].cluster_low;
                        tempIdx++;
                    }
                }
                idxQueue++;
            }

            if (finalPathCount == 0) {
                put("There are no folder or file with the name of that\n", LIGHT_RED);
            }
            break;
        default:
            put("error: too many arguments, expected 1 argument\n", LIGHT_RED);
            put("Usage: find <file/directory_name>\n", WHITE);
            break;
    }
}


void trackToParent(int currCluster) {
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
}
// int size(char str[]) {
//     int idx = 0;
//     int size = 0;
//     while (str[idx] != "\0") {
//         size++;
//     }
//     return size;
// }
// void BFS(char* goal) {
//     DynamicArray *Queue = createDynamicArray();
//     StringMatrix *QueuePath = createStringMatrix();
//     StringMatrix *finalPath = createStringMatrix();

//     int currCluster = 2;
//     struct FAT32DirectoryTable currDirectory;
//     add(Queue, currCluster);
//     int row = 0;
//     while (Queue->size != 0) {
//         currCluster = Queue->array[0];
//         syscall(9, &currDirectory, currCluster, 0);
//         if (row > 0) {
//             appendStringToRows(QueuePath, QueuePath->matrix[row - 1]);
//         }
//         appendString(QueuePath, row, currDirectory.table[0].name);
//         int index = 1;
//         while (currDirectory.table[index].user_attribute == UATTR_NOT_EMPTY) {
//             if (strcmp(currDirectory.table[index].name, goal)) {
//                 appendStringToRows(finalPath, QueuePath->matrix[row]);
//             }
//             add(Queue, currDirectory.table[index].cluster_low);
//             index++;
//         }
//         removeElementByIndex(Queue, 0);
//         row++;
//     }

//     freeDynamicArray(Queue);
//     freeMatrix(QueuePath);
// }