#include "header/scheduler/scheduler.h"
#include "header/stdlib/string.h"
#include "header/cpu/interrupt.h"

extern void process_context_switch(struct Context ctx);

void scheduler_init(void) {
    activate_timer_interrupt();
}

void scheduler_switch_to_next_process(void) {
    struct ProcessControlBlock* next_pcb = NULL;

    // Search for next process to be run using Round Robin algorithm
    int counter = 0;
    int next_index = (process_manager_state.running_process_idx + 1) % PROCESS_COUNT_MAX;
    while (next_index != process_manager_state.running_process_idx && counter != PROCESS_COUNT_MAX) {
        if (process_manager_state.process_list[next_index].metadata.state == READY) {
            break;
        }
        next_index = (next_index + 1) % PROCESS_COUNT_MAX;
        counter++;
    }

    // If there are no other processes waiting to be run, continue current process
    if (next_index == process_manager_state.running_process_idx) {
        process_manager_state.process_list[process_manager_state.running_process_idx].metadata.state = RUNNING;
        return;
    }
    // If there are no ready processes at all
    else if (counter == PROCESS_COUNT_MAX) {
        return;
    }

    // Switch to next process
    next_pcb = &(process_manager_state.process_list[next_index]);
    next_pcb->metadata.state = RUNNING;
    process_manager_state.running_process_idx = next_index;
    // if (next_pcb->context.eip > KERNEL_VIRTUAL_ADDRESS_BASE) {
    //     paging_use_page_directory(&_paging_kernel_page_directory);
    // } else {
    paging_use_page_directory(next_pcb->context.page_directory_virtual_addr);
    // }
    process_context_switch(next_pcb->context);
}

void scheduler_save_context_to_current_running_pcb(struct Context ctx) {
    struct ProcessControlBlock* current_pcb = process_get_current_running_pcb_pointer();
    if (!current_pcb) return;

    memcpy(&(current_pcb->context), &ctx, sizeof(struct Context));
    current_pcb->metadata.state = READY;
}