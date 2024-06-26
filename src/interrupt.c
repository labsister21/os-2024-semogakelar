#include "header/cpu/interrupt.h"
#include "header/cpu/portio.h"
#include "header/driver/keyboard.h"
#include "header/cpu/gdt.h"
#include "header/memory/paging.h"
#include "header/filesystem/fat32.h"
#include "header/driver/framebuffer.h"
#include "header/stdlib/string.h"
#include "header/driver/keyboard.h"
#include "header/scheduler/scheduler.h"
#include "header/cmos/cmos.h"

#define PIT_MAX_FREQUENCY   1193182
#define PIT_TIMER_FREQUENCY 1000
#define PIT_TIMER_COUNTER   (PIT_MAX_FREQUENCY / PIT_TIMER_FREQUENCY)

#define PIT_COMMAND_REGISTER_PIO          0x43
#define PIT_COMMAND_VALUE_BINARY_MODE     0b0
#define PIT_COMMAND_VALUE_OPR_SQUARE_WAVE (0b011 << 1)
#define PIT_COMMAND_VALUE_ACC_LOHIBYTE    (0b11  << 4)
#define PIT_COMMAND_VALUE_CHANNEL         (0b00  << 6) 
#define PIT_COMMAND_VALUE (PIT_COMMAND_VALUE_BINARY_MODE | PIT_COMMAND_VALUE_OPR_SQUARE_WAVE | PIT_COMMAND_VALUE_ACC_LOHIBYTE | PIT_COMMAND_VALUE_CHANNEL)

#define PIT_CHANNEL_0_DATA_PIO 0x40

void activate_timer_interrupt(void) {
    __asm__ volatile("cli");
    // Setup how often PIT fire
    uint32_t pit_timer_counter_to_fire = PIT_TIMER_COUNTER;
    out(PIT_COMMAND_REGISTER_PIO, PIT_COMMAND_VALUE);
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) (pit_timer_counter_to_fire & 0xFF));
    out(PIT_CHANNEL_0_DATA_PIO, (uint8_t) ((pit_timer_counter_to_fire >> 8) & 0xFF));

    // Activate the interrupt
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_TIMER));
}


void io_wait(void) {
    out(0x80, 0);
}

void pic_ack(uint8_t irq) {
    if (irq >= 8) out(PIC2_COMMAND, PIC_ACK);
    out(PIC1_COMMAND, PIC_ACK);
}

void pic_remap(void) {
    // Starts the initialization sequence in cascade mode
    out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); 
    io_wait();
    out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    out(PIC1_DATA, PIC1_OFFSET); // ICW2: Master PIC vector offset
    io_wait();
    out(PIC2_DATA, PIC2_OFFSET); // ICW2: Slave PIC vector offset
    io_wait();
    out(PIC1_DATA, 0b0100); // ICW3: tell Master PIC, slave PIC at IRQ2 (0000 0100)
    io_wait();
    out(PIC2_DATA, 0b0010); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    out(PIC1_DATA, ICW4_8086);
    io_wait();
    out(PIC2_DATA, ICW4_8086);
    io_wait();

    // Disable all interrupts
    out(PIC1_DATA, PIC_DISABLE_ALL_MASK);
    out(PIC2_DATA, PIC_DISABLE_ALL_MASK);
}

void activate_keyboard_interrupt(void) {
    out(PIC1_DATA, in(PIC1_DATA) & ~(1 << IRQ_KEYBOARD));
}

struct TSSEntry _interrupt_tss_entry = {
    .ss0  = GDT_KERNEL_DATA_SEGMENT_SELECTOR,
};

void set_tss_kernel_current_stack(void) {
    uint32_t stack_ptr;
    // Reading base stack frame instead esp
    __asm__ volatile ("mov %%ebp, %0": "=r"(stack_ptr) : /* <Empty> */);
    // Add 8 because 4 for ret address and other 4 is for stack_ptr variable
    _interrupt_tss_entry.esp0 = stack_ptr + 8; 
}

void interrupt(struct InterruptFrame frame) {
    int8_t ret_val = 0;
    switch (frame.cpu.general.eax) {
        case 0:
            *((int8_t*) frame.cpu.general.ecx) = read(*(struct FAT32DriverRequest*) frame.cpu.general.ebx);
            break;
        case 1:
            *((int8_t*) frame.cpu.general.ecx) = read_directory(*(struct FAT32DriverRequest*) frame.cpu.general.ebx);
            break;
        case 2:
            *((int8_t*) frame.cpu.general.ecx) = write(*(struct FAT32DriverRequest*) frame.cpu.general.ebx);    
            break;
        case 3:
            *((int8_t*) frame.cpu.general.ecx) = Delete(*(struct FAT32DriverRequest*) frame.cpu.general.ebx);    
            break;
        case 4:
            // while (is_keyboard_blocking()) {
            //     __asm__ volatile("cli");
            //     pic_ack(IRQ_TIMER);
            //     __asm__("sti");
            // }
            // char buf[KEYBOARD_BUFFER_SIZE] = {0};
            get_keyboard_buffer((char*) frame.cpu.general.ebx);
            // memcpy((char*) frame.cpu.general.ebx, buf, frame.cpu.general.ecx);
            break;
        case 5:
            putchar(*((char*)frame.cpu.general.ebx), frame.cpu.general.ecx);
            break;
        case 6:
            puts(
                (char*) frame.cpu.general.ebx, 
                frame.cpu.general.ecx, 
                frame.cpu.general.edx
            ); // Assuming puts() exist in kernel
            break;
        case 7: 
            keyboard_state_activate();
            break;
        case 8:
            framebuffer_clear();
            reset_write_position();
            break;
        case 9:
            read_clusters((struct FAT32DirectoryTable*) frame.cpu.general.ebx, frame.cpu.general.ecx, 1);
            break;
        case 10:
            write_clusters((struct FAT32DirectoryTable*) frame.cpu.general.ebx, frame.cpu.general.ecx, 1);
            break;
        case 11:
            terminate_current_process();
            break;
        case 12:
            ret_val = process_create_user_process(*(struct FAT32DriverRequest*) frame.cpu.general.ebx);
            *(int8_t*) frame.cpu.general.ecx = ret_val;
            break;
        case 13:
            print_active_processes();
            break;
        case 14:
            ret_val = kill_process((uint32_t) frame.cpu.general.ebx);
            *(int8_t*) frame.cpu.general.ecx = ret_val;
            break;
        case 15:
            print_current_time();
            break;
        case 16:
            keyboard_state_activate();
            break;
        case 17:
            framebuffer_write(frame.cpu.general.ecx, frame.cpu.general.edx, frame.cpu.general.ebx, 0xF, 0x0);
            break;  
    }
}

void main_interrupt_handler(struct InterruptFrame frame) {
    switch (frame.int_number) {
        case PAGE_FAULT:
            __asm__("hlt");
            break;        
        case IRQ_KEYBOARD + PIC1_OFFSET:
            keyboard_isr();
            break;
        case IRQ_TIMER + PIC1_OFFSET:
            struct Context current_ctx = {
                .cpu = frame.cpu,
                .eflags = frame.int_stack.eflags,
                .eip = frame.int_stack.eip,
                .page_directory_virtual_addr = paging_get_current_page_directory_addr()
            };
            scheduler_save_context_to_current_running_pcb(current_ctx);
            pic_ack(IRQ_TIMER);
            scheduler_switch_to_next_process();
            break;
        case 0x30:
            interrupt(frame);
            break;
    }
}