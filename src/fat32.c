#include <stdint.h>
#include <stdbool.h>
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"

static struct FAT32DriverState driver_state;

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster){
    strcpy(dir_table->table[0].name, ".");
    strcpy(dir_table->table[0].ext, "");
    dir_table->table[0].attribute = ATTR_SUBDIRECTORY;
    dir_table->table[0].user_attribute = UATTR_NOT_EMPTY;
    dir_table->table[0].undelete = false;
    // Set other fields accordingly
    dir_table->table[0].cluster_high = 0;
    dir_table->table[0].create_time = 0;
    dir_table->table[0].create_date = 0;
    dir_table->table[0].access_date = 0;
    dir_table->table[0].modified_time = 0;
    dir_table->table[0].modified_date = 0;
    dir_table->table[0].cluster_low = parent_dir_cluster; // Root directory cluster number
    dir_table->table[0].filesize = 0; // Directories have 0 filesize

    // Initialize ".." entry
    strcpy(dir_table->table[1].name, "..");
    strcpy(dir_table->table[1].ext, "");
    dir_table->table[1].attribute = ATTR_SUBDIRECTORY;
    dir_table->table[1].user_attribute = UATTR_NOT_EMPTY;
    dir_table->table[1].undelete = false;
    // Set other fields accordingly
    dir_table->table[1].cluster_high = 0; // Should be set to parent directory's cluster high
    dir_table->table[1].create_time = 0;
    dir_table->table[1].create_date = 0;
    dir_table->table[1].access_date = 0;
    dir_table->table[1].modified_time = 0;
    dir_table->table[1].modified_date = 0;
    dir_table->table[1].cluster_low = parent_dir_cluster; // Root directory cluster number
    dir_table->table[1].filesize = 0; // Directories have 0 filesize
}

void create_fat32(){
    struct BlockBuffer d;
    for (int i = 0; i < BLOCK_SIZE; i++) d.buf[i] = fs_signature[i];
    write_blocks(&d, BOOT_SECTOR, 0);
    driver_state.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    driver_state.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    driver_state.fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE;
    init_directory_table(&driver_state.dir_table_buf, "root", ROOT_CLUSTER_NUMBER);
}

bool is_empty_storage(){
    struct BlockBuffer boot_sector;
    read_blocks(&boot_sector, BOOT_SECTOR, 1);
    return (memcmp(boot_sector.buf, fs_signature, BLOCK_SIZE) != 0);
}

void initialize_filesystem_fat32(){
    if (is_empty_storage()){
        create_fat32();
    } else {
        read_clusters(&driver_state, 1, 3);
    }
}

uint32_t cluster_to_lba(uint32_t cluster){
    return ((cluster) * 4) + 1;
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    uint32_t logical_block_address = cluster_to_lba(cluster_number);
    uint8_t block_count = cluster_count * 4;
    read_blocks(&ptr, logical_block_address, block_count);
}

void write_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    uint32_t logical_block_address = cluster_to_lba(cluster_number);
    uint8_t block_count = cluster_count * 4;
    write_blocks(&ptr, logical_block_address, block_count);
}