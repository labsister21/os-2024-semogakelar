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
    // hitung nomor cluster
    uint32_t empty_cluster = 0;
    char* ext = "/0";
    while (driver_state.fat_table.cluster_map[empty_cluster] != FAT32_FAT_EMPTY_ENTRY) {
        empty_cluster++;
    }

    memcpy(dir_table->table[0].name, name, sizeof(name));
    memcpy(dir_table->table[0].ext, ext, sizeof(dir_table->table[0].ext));
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
    dir_table->table[0].cluster_low = empty_cluster; // Root directory cluster number
    dir_table->table[0].filesize = 0; // Directories have 0 filesize

    // Initialize ".." entry

    char* parent_name;
    char* root = "root";
    if ( memcmp(name, root, 4) == 0){
        parent_name = "root";
    } else {
        int dir_idx = 1;
        struct ClusterBuffer c;
        read_clusters (&c, parent_dir_cluster, 1);
        memcpy(parent_name, &c + 32, 8);
    }

    memcpy(dir_table->table[1].name, parent_name, sizeof(parent_name));
    memcpy(dir_table->table[0].ext, "\0", sizeof(dir_table->table[0].ext));
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
    write_blocks(&d, BOOT_SECTOR, 1);
    memset(driver_state.fat_table.cluster_map, FAT32_FAT_EMPTY_ENTRY, CLUSTER_SIZE);
    driver_state.fat_table.cluster_map[0] = CLUSTER_0_VALUE;
    driver_state.fat_table.cluster_map[1] = CLUSTER_1_VALUE;
    init_directory_table(&driver_state.dir_table_buf, "root", ROOT_CLUSTER_NUMBER);
    driver_state.fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE;

    struct ClusterBuffer b;
    memcpy(&b, &driver_state.dir_table_buf, sizeof(driver_state.dir_table_buf.table));
    write_clusters(&b, 2, 1);

    memcpy(&b, &driver_state.fat_table, sizeof(driver_state.fat_table.cluster_map));
    write_clusters(&b, 1, 1);
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
        read_clusters(&driver_state.cluster_buf, cluster_to_lba(1), 1);
    }
}

uint32_t cluster_to_lba(uint32_t cluster){
    return ((cluster) * 4);
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    uint32_t logical_block_address = cluster_to_lba(cluster_number);
    uint8_t block_count = cluster_count * 4;
    for (int i = 0; i < 4*cluster_count; i++) {
        read_blocks(ptr+(BLOCK_SIZE*i*8), logical_block_address+(CLUSTER_SIZE*i), block_count);
    }
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count){
    uint32_t logical_block_address = cluster_to_lba(cluster_number);
    uint8_t block_count = cluster_count * 4;
    for (int i = 0; i < 4*cluster_count; i++) {
        write_blocks(ptr+(BLOCK_SIZE*i*8), logical_block_address+(CLUSTER_SIZE*i), block_count);
    }
}

// int8_t read_directory(struct FAT32DriverRequest request){

// }

// int8_t read(struct FAT32DriverRequest request){
//     if (driver_state.fat_table.cluster_map[request.parent_cluster_number] == FAT32_FAT_END_OF_FILE) {
//         return 1;
//     }
    
//     int dir_idx = 1;
//     while (driver_state.dir_table_buf.table[dir_idx].cluster_low != request.parent_cluster_number){
//         dir_idx++;
//     }
// }

// int8_t write(struct FAT32DriverRequest request){

// }

// int8_t Delete(struct FAT32DriverRequest request){

// }