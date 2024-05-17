#include "header/cmos/cmos.h"
#include "header/driver/framebuffer.h"

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

// int main() {
//     while (true) {
//         framebuffer_write(20, 20, 'h', 0xF, 0x00);
//         framebuffer_write(20, 21, 'e', 0xF, 0x00);
//         framebuffer_write(20, 22, 'l', 0xF, 0x00);
//         framebuffer_write(20, 23, 'l', 0xF, 0x00);
//         framebuffer_write(20, 24, 'o', 0xF, 0x00);
//         // framebuffer_write(20, 20, ' ', 0x0, 0x00);
//         // framebuffer_write(20, 21, ' ', 0x0, 0x00);
//         // framebuffer_write(20, 22, ' ', 0x0, 0x00);
//         // framebuffer_write(20, 23, ' ', 0x0, 0x00);
//         // framebuffer_write(20, 24, ' ', 0x0, 0x00);   
//     }

//     return 0;
// }