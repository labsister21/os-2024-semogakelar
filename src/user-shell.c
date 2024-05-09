#include <stdint.h>
#include <stdbool.h>
#include "header/shell/user-shell.h"
#include "header/shell/cd.h"

uint32_t current_directory = ROOT_CLUSTER_NUMBER;
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

void put(char* str, uint8_t color) {
    syscall(6, (uint32_t) str, strlen(str), color);
}

void print_current_directory() {
    put("/", LIGHT_BLUE);
    if (current_directory == ROOT_CLUSTER_NUMBER) {
        return;
    }
}

void update_current_directory(char* directory_name, uint32_t parent_cluster_number) {
    
    struct FAT32DriverRequest request = {
        .buf                    = &current_dir_table,
        .ext                    = "\0\0\0",
        .parent_cluster_number  = parent_cluster_number,
        .buffer_size            = sizeof(current_dir_table)
    };
    memcpy(request.name, directory_name, 8);
    int8_t ret_val;
    syscall(1, (uint32_t) &request, (uint32_t) &ret_val, 0);
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

void parse_path(char* str, char paths[][512], uint32_t* num_of_directory) {
    uint8_t num_of_folder = 1;
    uint32_t idx = 0;
    while (str[idx] != '\0') {
        if (str[idx] == '/' || str[idx] == '\\') {
            num_of_folder++;
        }
        idx++;
    }
    *num_of_directory = num_of_folder;
    idx = 0;

    uint32_t path_idx = 0, path_cnt = 0;
    while (str[idx] != '\0') {
        if (str[idx] == '/' || str[idx] == '\\') {
            path_cnt++;
            path_idx = 0;
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
    update_current_directory("root\0\0\0\0", ROOT_CLUSTER_NUMBER);

    while (true) {
        clear(buf_input, 2048);
        clear(cmd, 512);
        clear(args, 1024);
        args_count = 0;

        put("os2024-semogakelar", LIGHT_GREEN);
        put(":", GREY);
        print_current_directory();
        put("$ ", GREY);

        syscall(4, (uint32_t) buf_input, 2048, 0);
        
        int8_t ret_val = parse_input(buf_input, cmd, args, &args_count);
        if (ret_val != 0) {
            put("error: too many arguments\n", RED);
        } 
        else if (memcmp(cmd, "cd", strlen(cmd)) == 0) {
            cd(args, args_count);
        }
        else {
            put("error: no such command called '", RED);
            put(cmd, RED);
            put("'\n", RED);
        }
    }

    return 0;
}
