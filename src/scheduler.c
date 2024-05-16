#include "header/scheduler/scheduler.h"
#include "header/stdlib/string.h"

extern void process_context_switch(struct Context ctx);

void scheduler_init(void) {
    // Inisialisasi semua entri dalam daftar proses menjadi tidak aktif
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        process_manager_state.process_list[i].metadata.state = NEW;
    }
    // Atur jumlah proses aktif menjadi 0
    process_manager_state.active_process_count = 0;
}

void scheduler_switch_to_next_process(void) {
    struct ProcessControlBlock* current_pcb = process_get_current_running_pcb_pointer();
    struct ProcessControlBlock* next_pcb = NULL;

    // Select the next process to run (Round Robin)
    int next_index = -1;
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        if (process_manager_state.process_list[i].metadata.state == READY) {
            next_index = i;
            break;
        }
    }

    if (next_index != -1) {
        next_pcb = &(process_manager_state.process_list[next_index]);
        next_pcb->metadata.state = RUNNING;

        if (current_pcb) {
            struct Context ctx = {0};
            __asm__ volatile("mov %%edi, %0" : "=r"(ctx.cpu.index.edi): /* <Empty> */);
            __asm__ volatile("mov %%esi, %0" : "=r"(ctx.cpu.index.esi): /* <Empty> */);

            __asm__ volatile("mov %%ebp, %0" : "=r"(ctx.cpu.stack.ebp): /* <Empty> */);
            __asm__ volatile("mov %%esp, %0" : "=r"(ctx.cpu.stack.esp): /* <Empty> */);

            __asm__ volatile("mov %%eax, %0" : "=r"(ctx.cpu.general.eax): /* <Empty> */);
            __asm__ volatile("mov %%ebx, %0" : "=r"(ctx.cpu.general.ebx): /* <Empty> */);
            __asm__ volatile("mov %%ecx, %0" : "=r"(ctx.cpu.general.ecx): /* <Empty> */);
            __asm__ volatile("mov %%edx, %0" : "=r"(ctx.cpu.general.edx): /* <Empty> */);

            __asm__ volatile("mov %%gs, %0" : "=r"(ctx.cpu.segment.gs): /* <Empty> */);
            __asm__ volatile("mov %%fs, %0" : "=r"(ctx.cpu.segment.fs): /* <Empty> */);
            __asm__ volatile("mov %%es, %0" : "=r"(ctx.cpu.segment.es): /* <Empty> */);
            __asm__ volatile("mov %%ds, %0" : "=r"(ctx.cpu.segment.ds): /* <Empty> */);

            __asm__ volatile("movl $0f, %0\n\t" : "=r"(ctx.eip)); // $0f adalah label yang diletakkan setelah instruksi ini
            __asm__ volatile("pushf\n\t"
                             "pop %0\n\t" : "=r"(ctx.eflags));

            ctx.page_directory_virtual_addr = current_pcb->context.page_directory_virtual_addr;

            memcpy(&(current_pcb->context), &ctx, sizeof(struct Context));
            current_pcb->metadata.state = READY;
            
            paging_use_page_directory(next_pcb->context.page_directory_virtual_addr);
        }
        process_context_switch(next_pcb->context);
    }    
}

void scheduler_save_context_to_current_running_pcb(struct Context ctx) {
    struct ProcessControlBlock* current_pcb = process_get_current_running_pcb_pointer();
    memcpy(&(current_pcb->context), &ctx, sizeof(struct Context));
}