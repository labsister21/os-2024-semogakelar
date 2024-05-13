#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"

struct ProcessManagerState process_manager_state = {
    .active_process_count = 0
};

struct ProcessControlBlock _process_list[PROCESS_COUNT_MAX] = {0};

struct ProcessControlBlock* process_get_current_running_pcb_pointer(void) {
    int i = 0;
    while (i < PROCESS_COUNT_MAX && _process_list[i].metadata.state != RUNNING) {
        i++;
    }

    if (i == PROCESS_COUNT_MAX) return NULL;
    return &_process_list[i];
}

int32_t process_create_user_process(struct FAT32DriverRequest request) {
    int32_t retcode = PROCESS_CREATE_SUCCESS; 
    if (process_manager_state.active_process_count >= PROCESS_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_MAX_PROCESS_EXCEEDED;
        goto exit_cleanup;
    }

    // Ensure entrypoint is not located at kernel's section at higher half
    if ((uint32_t) request.buf >= KERNEL_VIRTUAL_ADDRESS_BASE) {
        retcode = PROCESS_CREATE_FAIL_INVALID_ENTRYPOINT;
        goto exit_cleanup;
    }

    // Check whether memory is enough for the executable and additional frame for user stack
    uint32_t page_frame_count_needed = ceil_div(request.buffer_size + PAGE_FRAME_SIZE, PAGE_FRAME_SIZE);
    if (!paging_allocate_check(page_frame_count_needed) || page_frame_count_needed > PROCESS_PAGE_FRAME_COUNT_MAX) {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    // Process PCB 
    int32_t p_index = process_list_get_inactive_index();
    struct ProcessControlBlock *new_pcb = &(_process_list[p_index]);

    new_pcb->metadata.pid = process_generate_new_pid();

exit_cleanup:
    return retcode;
}

uint32_t process_generate_new_pid() {
    uint32_t pid = 1;
    bool found = false;
    while (!found) {
        int idx = 0;
        while (idx < PROCESS_COUNT_MAX && _process_list[idx].metadata.pid != pid) {
            idx++;
        }
        if (idx == PROCESS_COUNT_MAX)
            found = true;
        else {
            pid++;
        }
    }

    process_manager_state.active_process_count++;
    return pid;
}

int32_t process_list_get_inactive_index() {
    int32_t i = 0;
    while (_process_list[i].metadata.state != NEW) {
        i++;
    }
    return i;
}

bool process_destroy(uint32_t pid) {
    int i = 0;
    while (_process_list[i].metadata.pid != pid && i < PROCESS_COUNT_MAX) i++;

    if (i == PROCESS_COUNT_MAX) return false;

    memset(&_process_list[i], 0, sizeof(struct ProcessControlBlock));
    return true;
}

uint32_t ceil_div(uint32_t a, uint32_t b) {
    uint32_t result = a / b;

    if (a % b != 0) {
        result++;
    }

    return result;
}