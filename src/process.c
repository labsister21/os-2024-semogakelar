#include "header/process/process.h"
#include "header/memory/paging.h"
#include "header/stdlib/string.h"
#include "header/cpu/gdt.h"
#include "header/driver/keyboard.h"
#include "header/shell/cp.h"

struct ProcessManagerState process_manager_state = {
    .process_list = {0},
    .active_process_count = 0,
    .running_process_idx = -1
};

struct ProcessControlBlock* process_get_current_running_pcb_pointer(void) {
    if (process_manager_state.running_process_idx == -1)
        return NULL;
    
    return &(process_manager_state.process_list[process_manager_state.running_process_idx]);
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

    // Search for empty PCB slot
    int32_t p_index = process_list_get_inactive_index();
    struct ProcessControlBlock* new_pcb = &(process_manager_state.process_list[p_index]);

    // Create new virtual address space using page directory
    struct PageDirectory* new_page = paging_create_new_page_directory();
    if (!new_page) {
        retcode = PROCESS_CREATE_FAIL_NOT_ENOUGH_MEMORY;
        goto exit_cleanup;
    }

    // if ()

    for (uint32_t i = 0; i < page_frame_count_needed; i++) {
        void* virtual_addr = (void*) (i * PAGE_FRAME_SIZE);
        paging_allocate_user_page_frame(new_page, virtual_addr);
        new_pcb->memory.virtual_addr_used[i] = virtual_addr;
    }
    new_pcb->memory.page_frame_used_count = page_frame_count_needed;
    
    // Read executable file from filesystem
    struct PageDirectory* temp_page = paging_get_current_page_directory_addr();
    paging_use_page_directory(new_page);

    int8_t ret_val = read(request);
    if (ret_val != 0) {
        retcode = PROCESS_CREATE_FAIL_FS_READ_FAILURE;
        paging_use_page_directory(temp_page);
        goto exit_cleanup;
    }


    // NTAR DIGANTI HEHE
    memcpy(new_pcb->metadata.process_name, request.name, 8);
    // for (uint32_t i = 0; i < new_pcb->memory.page_frame_used_count; i++) {
    memcpy(new_pcb->memory.virtual_addr_used[0], request.buf, PAGE_FRAME_SIZE);
    memset(new_pcb->memory.virtual_addr_used[1], 0, PAGE_FRAME_SIZE);
    // }

    paging_use_page_directory(temp_page);
    

    // Initialize process context
    struct CPURegister new_register_state = {
        .index = {0},
        .stack = {
            .ebp = 0x400000 - 4,
            .esp = 0x400000 - 4
        },
        .general = {0},
        .segment = {
            .gs = 0x3 | (0x4 << 3),
            .fs = 0x3 | (0x4 << 3),
            .es = 0x3 | (0x4 << 3),
            .ds = 0x3 | (0x4 << 3)
        }
    };
    
    memcpy(&(new_pcb->context.cpu), &new_register_state, sizeof(struct CPURegister));
    new_pcb->context.eip = 0;
    new_pcb->context.eflags = (uint16_t) CPU_EFLAGS_BASE_FLAG | CPU_EFLAGS_FLAG_INTERRUPT_ENABLE;
    new_pcb->context.page_directory_virtual_addr = new_page;

    // Initialize process metadata
    new_pcb->metadata.pid = process_generate_new_pid();
    new_pcb->metadata.state = READY;

    process_manager_state.active_process_count++;

    goto exit_cleanup;

exit_cleanup:
    return retcode;
}

uint32_t process_generate_new_pid() {
    uint32_t pid = 1;
    bool found = false;
    while (!found) {
        int idx = 0;
        while (idx < PROCESS_COUNT_MAX && process_manager_state.process_list[idx].metadata.pid != pid) {
            idx++;
        }
        if (idx == PROCESS_COUNT_MAX)
            found = true;
        else {
            pid++;
        }
    }
    return pid;
}

int32_t process_list_get_inactive_index() {
    int32_t i = 0;
    while (process_manager_state.process_list[i].metadata.state != NEW) {
        i++;
    }
    return i;
}

bool process_destroy(uint32_t pid) {
    int i = 0;
    while (process_manager_state.process_list[i].metadata.pid != pid && i < PROCESS_COUNT_MAX) i++;

    if (i == PROCESS_COUNT_MAX) return false;

    memset(&(process_manager_state.process_list[i]), 0, sizeof(struct ProcessControlBlock));
    return true;
}

uint32_t ceil_div(uint32_t a, uint32_t b) {
    uint32_t result = a / b;

    if (a % b != 0) {
        result++;
    }

    return result;
}

void terminate_current_process() {
    struct ProcessControlBlock* current_pcb = process_get_current_running_pcb_pointer();
    memset(current_pcb, 0, sizeof(struct ProcessControlBlock));
    scheduler_switch_to_next_process();
}

void print_active_processes() {
    char* str = "Currently running processes:\n";
    puts(str, strlen(str), 0b0111);
    for (int i = 0; i < PROCESS_COUNT_MAX; i++) {
        if (process_manager_state.process_list[i].metadata.state == RUNNING || process_manager_state.process_list[i].metadata.state == READY) {
            puts("PID: ", strlen("PID: "), 0b0111);
            char* pid = itoa(process_manager_state.process_list[i].metadata.pid, 10);
            puts(pid, strlen(pid), 0b1111);
            puts("     Name: ", strlen("     Name: "), 0b0111);
            puts(process_manager_state.process_list[i].metadata.process_name, strlen(process_manager_state.process_list[i].metadata.process_name), 0b1111);
            puts("\n", strlen("\n"), 0b1111);
        }
    }
}

int8_t kill_process(uint32_t pid) {
    if (process_manager_state.process_list[process_manager_state.running_process_idx].metadata.pid == pid) {
        terminate_current_process();
        return 0;
    }
    
    int i = 0;
    while (i < PROCESS_COUNT_MAX && process_manager_state.process_list[i].metadata.pid != pid) {
        i++;
    }
    if (i == PROCESS_COUNT_MAX) {
        return -1;
    }

    memset(&(process_manager_state.process_list[i]), 0, sizeof(struct ProcessControlBlock));
    return 0;
}