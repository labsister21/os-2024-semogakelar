#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/memory/paging.h"

__attribute__((aligned(0x1000))) struct PageDirectory _paging_kernel_page_directory = {
    .table = {
        [0] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address          = 0,
        },
        [0x300] = {
            .flag.present_bit       = 1,
            .flag.write_bit         = 1,
            .flag.use_pagesize_4_mb = 1,
            .lower_address          = 0,
        }
    }
};

static struct PageManagerState page_manager_state = {
    .page_frame_map = {
        [0]                            = true,
        [1 ... PAGE_FRAME_MAX_COUNT-1] = false
    },
    .free_page_frame_count = PAGE_FRAME_MAX_COUNT - 1
};

void update_page_directory_entry(
    struct PageDirectory *page_dir,
    void *physical_addr, 
    void *virtual_addr, 
    struct PageDirectoryEntryFlag flag
) {
    uint32_t page_index = ((uint32_t) virtual_addr >> 22) & 0x3FF;
    page_dir->table[page_index].flag          = flag;
    page_dir->table[page_index].lower_address = (( uint32_t) physical_addr >> 22) & 0x3FF;
    flush_single_tlb(virtual_addr);
}

void flush_single_tlb(void *virtual_addr) {
    asm volatile("invlpg (%0)" : /* <Empty> */ : "b"(virtual_addr): "memory");
}



/* --- Memory Management --- */
// TODO: Implement
bool paging_allocate_check(uint32_t amount) {
    // TODO: Check whether requested amount is available
    uint32_t required_number_of_pages = (amount / PAGE_FRAME_SIZE) + ((amount % PAGE_FRAME_SIZE > 0)? 1 : 0);
    return page_manager_state.free_page_frame_count >= required_number_of_pages;
}


bool paging_allocate_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    /*
     * TODO: Find free physical frame and map virtual frame into it
     * - Find free physical frame in page_manager_state.page_frame_map[] using any strategies
     * - Mark page_manager_state.page_frame_map[]
     * - Update page directory with user flags:
     *     > present bit    true
     *     > write bit      true
     *     > user bit       true
     *     > pagesize 4 mb  true
     */ 
    
    // If there's no free physical frame left
    if (!paging_allocate_check(PAGE_FRAME_SIZE)) {
        return false;
    }

    // Find free physical frame using first-fit algorithm
    uint32_t i = 0;
    while (i < PAGE_FRAME_MAX_COUNT && page_manager_state.page_frame_map[i]) {
        i++;
    }
    
    struct PageDirectoryEntryFlag flag = {
        .present_bit        = 1,
    // TODO : Continue. Note: Only 8-bit flags
        .write_bit          = 1,
        .user_supervisor_bit= 1,
        .pwt_bit            = 0,
        .pcd_bit            = 0,
        .accesed_bit        = 0,
        .dirty_bit          = 0,
        .use_pagesize_4_mb  = 1
    };
    
    update_page_directory_entry(page_dir, (void *)(i * PAGE_FRAME_SIZE), virtual_addr, flag);
    page_manager_state.page_frame_map[i] = true;
    page_manager_state.free_page_frame_count--;

    return true;
}

bool paging_free_user_page_frame(struct PageDirectory *page_dir, void *virtual_addr) {
    /* 
     * TODO: Deallocate a physical frame from respective virtual address
     * - Use the page_dir.table values to check mapped physical frame
     * - Remove the entry by setting it into 0
     */
    uint32_t page_index = ((uint32_t)virtual_addr >> 22) & 0x3FF;

    struct PageDirectoryEntry *page_entry = &page_dir->table[page_index];

    // If the page hasn't been allocated
    if (!page_entry->flag.present_bit)
        return false;

    struct PageDirectoryEntry empty_entry = {0};
    *page_entry = empty_entry;
    page_manager_state.page_frame_map[page_entry->lower_address] = false;
    page_manager_state.free_page_frame_count++;

    return true;
}