#include <stdbool.h>
#include "header/stdlib/string.h"

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

int main() {
    char* str = "Clock is running...\n";
    syscall(6, (uint32_t) str, strlen(str), 0b1111);
    while (true) {
        syscall(15, 0, 0, 0);
        syscall(16, 0, 0, 0);
    }

    return 0;
}