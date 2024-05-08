#include <stdint.h>
#include "user-shell.h"


uint32_t current_directory = ROOT_CLUSTER_NUMBER;

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

int main(void) {
    
    char args[2048];
    // uint8_t args_len;

    while (true) {
        clear(args, 2048);
        // args_len = 0;

        put("os2024-semogakelar\0", LIGHT_GREEN);
        put(":", GREY);
        put("/", LIGHT_BLUE);
        put("$ ", GREY);

        syscall(4, (uint32_t) args, 2048, 0);
    }

    return 0;
}
