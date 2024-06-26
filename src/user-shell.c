#include <stdint.h>
#include <stdbool.h>
#include "header/shell/user-shell.h"
#include "header/shell/cd.h"
#include "header/shell/mkdir.h"
#include "header/shell/ls.h"
#include "header/shell/cat.h"
#include "header/shell/cp.h"
#include "header/shell/rm.h"
#include "header/shell/find.h"
#include "header/shell/mv.h"
#include "header/shell/exec.h"

uint32_t current_directory = ROOT_CLUSTER_NUMBER;
char current_path[512];
struct FAT32DirectoryTable current_dir_table = {0};

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void ps(int args_count) {
    switch (args_count) {
        case 0:
            syscall(13, 0, 0, 0);
            break;
        default:
            put("error: too many arguments, expected 0 argument\n", LIGHT_RED);
            put("Usage: ps\n", LIGHT_GREY);
            break;
    }
}

void kill(char args[][512], int args_count) {
    switch (args_count) {
        case 0:
            put("error: missing argument\n", LIGHT_RED);
            put("Usage: kill <active_process_pid>\n", LIGHT_GREY);
            break;
        case 1:
            if (strlen(args[0]) > 2 || !(args[0][0] > '0' && args[0][0] <= '9') || (strlen(args[0]) == 2 && !(args[0][1] > '0' && args[0][1] <= '9'))) {
                put("error: invalid argument\n", LIGHT_RED);
                put("Usage: kill <active_process_pid>\n", LIGHT_GREY);
                return;
            }

            int8_t pid = args[0][0] - '0';
            if (strlen(args[0]) > 2) {
                pid *= 10;
                pid += args[0][1] - '0';
            }

            int8_t ret_val = 0;
            syscall(14, pid, (uint32_t) &ret_val, 0);
            if (ret_val != 0) {
                put("error: no such active process with that pid\n", LIGHT_RED);
                return;
            }
            break;
        default:
            put("error: too many arguments, expected 1 argument\n", LIGHT_RED);
            put("Usage: kill <active_process_pid>\n", LIGHT_GREY);
            break;
    }
}

void put(char* str, uint8_t color) {
    syscall(6, (uint32_t) str, strlen(str), color);
}

void get_input(char* buf) {
    int idx = 0;
    while (true) {
        syscall(4, (uint32_t) &buf[idx], 0, 0);
        if (buf[idx] == '\b') {
            buf[idx] = '\0';
            if (idx > 0) {
                idx--;
                buf[idx] = '\0';
            }
        }
        else if (buf[idx] == '\n') {
            buf[idx] = '\0';
            break;
        }
        else if (buf[idx] == '\t') {
            buf[idx] = ' '; idx++;
            buf[idx] = ' '; idx++;
        }
        else if (buf[idx] != '\0') {
            idx++;
        }
    }
}

void print_path(char path[][512], uint32_t idx, uint8_t color) {
    if (path[0][0] != '\0')
        put(path[0], color);
    
    for (uint32_t i = 1; i <= idx; i++) {
        put("/", color);
        put(path[i], color);
    }
}

void update_directory_table(struct FAT32DirectoryTable* dir_table, uint32_t directory_cluster_number) {
    syscall(9, (uint32_t) dir_table, directory_cluster_number, 0);
}

void ignore_blanks(char* str, uint32_t* idx) {
    while (str[*idx] == ' ') {
        (*idx)++;
    }
}

void copy_word(char* str, char* dest, uint32_t* idx) {
    uint32_t i = 0;
    while (str[*idx] != ' ' && str[*idx] != '\0') {
        dest[i] = str[*idx];
        (*idx)++; i++;
    }
}

void append_current_path(char* path) {
    if (strlen(current_path) != 1) current_path[strlen(current_path)] = '/';
    int32_t current_path_len = strlen(current_path);
    for (size_t i = 0; i < strlen(path); i++) {
        current_path[i + current_path_len] = path[i];
    }
}

void retract_current_path() {
    int i = strlen(current_path) - 1;
    while (current_path[i] != '/') {
        current_path[i] = '\0';
        i--;
    }
    if (i > 0) {
        current_path[i] = '\0';
    }
}

void parse_path(char* str, char paths[][512], uint32_t* num_of_directory) {
    // Handle absolute path
    uint32_t idx = 0;
    if (str[0] == '/' || str[0] == '\\') {
        update_directory_table(&current_dir_table, ROOT_CLUSTER_NUMBER);
        current_directory = ROOT_CLUSTER_NUMBER;
        memset(current_path, 0, 512);
        current_path[0] = '/';
        idx++;
    }
    
    uint8_t num_of_folder = 1;
    while (str[idx] != '\0') {
        if ((str[idx] == '/' || str[idx] == '\\') &&
            (strlen(str) > idx+1 && (str[idx+1] != '\0' || str[idx+1] != ' '))) {
            
            num_of_folder++;
        }
        idx++;
    }
    *num_of_directory = num_of_folder;
    idx = (str[0] == '/' || str[0] == '\\')? 1 : 0;

    uint32_t path_idx = 0, path_cnt = 0;
    while (str[idx] != '\0') {
        if (str[idx] == '/' || str[idx] == '\\') {
            if (strlen(str) > idx+1 && (str[idx+1] != '\0' || str[idx+1] != ' ')) {    
                path_cnt++;
                path_idx = 0;
            }
        }
        else {
            paths[path_cnt][path_idx] = str[idx];
            path_idx++;
        }
        idx++;
    }
}

int8_t parse_input(char* buf_input, char* cmd, char args[2][512], uint8_t* args_count) {
    uint32_t buf_idx = 0;
    
    ignore_blanks(buf_input, &buf_idx);
    copy_word(buf_input, cmd, &buf_idx);

    ignore_blanks(buf_input, &buf_idx);
    copy_word(buf_input, args[0], &buf_idx);

    ignore_blanks(buf_input, &buf_idx);
    copy_word(buf_input, args[1], &buf_idx);

    for (int i = 0; i < 2; i++) {
        if (args[i][0] != '\0')
            (*args_count)++;
    }

    // If there are more than twp arguments, return error code
    while (buf_input[buf_idx] != '\0') {
        if (buf_input[buf_idx] != ' ') {
            return -1;
        }
        buf_idx++;
    }

    return 0;    
}

int main(void) {
    char buf_input[2048];
    char cmd[512];
    char args[2][512];
    uint8_t args_count;

    memset((void*) &current_dir_table, 0, sizeof(struct FAT32DirectoryTable));
    update_directory_table(&current_dir_table, ROOT_CLUSTER_NUMBER);

    current_path[0] = '/';
    for (int i = 1; i < 512; i++)
        current_path[i] = '\0';

    while (true) {
        strclear(buf_input, 2048);
        strclear(cmd, 512);
        strclear(args, 1024);
        args_count = 0;

        put("os2024-semogakelar", LIGHT_GREEN);
        put(":", DARK_GREY);
        put(current_path, LIGHT_BLUE);
        put("$ ", DARK_GREY);

        syscall(7, 0, 0, 0);
        get_input(buf_input);
        
        int8_t ret_val = parse_input(buf_input, cmd, args, &args_count);
        if (ret_val != 0) {
            put("error: too many arguments\n", LIGHT_RED);
        } 
        else if (strlen(cmd) == 2 && memcmp(cmd, "cd", 2) == 0) {
            cd(args, args_count);
        }
        else if (strlen(cmd) == 5 && memcmp(cmd, "clear", 5) == 0) {
            if (args[0][0] != '\0') {
                put("error: too many arguments, expected 0 argument\n", LIGHT_RED);
                put("Usage: clear\n", LIGHT_GREY);
            }
            else {
                syscall(8, 0, 0, 0);
            }
        }
        else if (strlen(cmd) == 2 && memcmp(cmd, "ls", 2) == 0) {
            ls(args_count);
        }
        else if (strlen(cmd) == 5 && memcmp(cmd, "mkdir", 5) == 0) {
            mkdir(args, args_count);
        }
        else if (strlen(cmd) == 3 && memcmp(cmd, "cat", 3) == 0) {
            cat(args, args_count);
        }
        else if (strlen(cmd) == 2 && memcmp(cmd, "cp", 2) == 0) {
            cp(args, args_count);
        }
        else if (strlen(cmd) == 2 && memcmp(cmd, "rm", 2) == 0) {
            rm(args, args_count);
        }
        else if (strlen(cmd) == 4 && memcmp(cmd, "find", 4) == 0) {
            find(args, args_count);
        }
        else if (strlen(cmd) == 2 && memcmp(cmd, "mv", 2) == 0) {
            mv(args, args_count);
        }
        else if (strlen(cmd) == 4 && memcmp(cmd, "exec", 2) == 0) {
            exec(args, args_count);
        }
        else if (strlen(cmd) == 2 && memcmp(cmd, "ps", 2) == 0) {
            ps(args_count);
        }
        else if (strlen(cmd) == 4 && memcmp(cmd, "kill", 2) == 0) {
            kill(args, args_count);
        }
        else if (strlen(cmd) == 5 && memcmp(cmd, "clock", 5) == 0) {
            syscall(15, 0, 0, 0);
        }
        else if (!(cmd[0] == '\0')) {
            put("error: no such command with the name '", LIGHT_RED);
            put(cmd, LIGHT_RED);
            put("'\n", LIGHT_RED);
        }
    }

    return 0;
}
