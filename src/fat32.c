#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"
#include "header/driver/disk.h"

struct FAT32DriverState fat32_driver_state;

const uint8_t fs_signature[BLOCK_SIZE] = {
    'C', 'o', 'u', 'r', 's', 'e', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',  ' ',
    'D', 'e', 's', 'i', 'g', 'n', 'e', 'd', ' ', 'b', 'y', ' ', ' ', ' ', ' ',  ' ',
    'L', 'a', 'b', ' ', 'S', 'i', 's', 't', 'e', 'r', ' ', 'I', 'T', 'B', ' ',  ' ',
    'M', 'a', 'd', 'e', ' ', 'w', 'i', 't', 'h', ' ', '<', '3', ' ', ' ', ' ',  ' ',
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '2', '0', '2', '4', '\n',
    [BLOCK_SIZE-2] = 'O',
    [BLOCK_SIZE-1] = 'k',
};

uint32_t cluster_to_lba(uint32_t cluster) {
    return CLUSTER_BLOCK_COUNT * cluster;
}

void init_directory_entry(struct FAT32DirectoryEntry* dest, char* name, char* ext, uint8_t attribute, uint32_t cluster_number, uint32_t filesize) {
    memcpy(dest->name, name, sizeof(dest->name));
    memcpy(dest->ext, ext, sizeof(dest->ext));
    dest->attribute = attribute;
    dest->user_attribute = UATTR_NOT_EMPTY;
    dest->cluster_low = cluster_number & 0xFFFF;
    dest->cluster_high = (cluster_number >> 16) & 0xFFFF;
    dest->filesize = filesize;
}

uint32_t find_empty_cluster(struct FAT32FileAllocationTable* fat_table) {
    uint32_t idx = 0;
    while (idx < CLUSTER_MAP_SIZE && fat_table->cluster_map[idx] != FAT32_FAT_EMPTY_ENTRY) 
        idx++;

    if (idx < CLUSTER_MAP_SIZE)
        return idx;
    else
        return -1;
}

uint32_t find_empty_dir_entry(struct FAT32DirectoryTable* dir_table) {
    uint32_t idx = 0;
    while (idx < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) && dir_table->table[idx].attribute == UATTR_NOT_EMPTY)
        idx++;

    if (idx < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry))
        return idx;
    else
        return -1;
}

bool enough_empty_cluster(struct FAT32FileAllocationTable* fat_table, uint32_t required_empty_clusters) {
    uint32_t idx = 2, number_of_empty_clusters = 0;
    while (idx < CLUSTER_MAP_SIZE && number_of_empty_clusters < required_empty_clusters) {
        idx++;
        if (fat_table->cluster_map[idx] == FAT32_FAT_EMPTY_ENTRY)
            number_of_empty_clusters++;
    }
    return number_of_empty_clusters == required_empty_clusters;
}

void init_directory_table(struct FAT32DirectoryTable *dir_table, char *name, uint32_t parent_dir_cluster) {
    struct FAT32DirectoryEntry empty_entry = {0};
    for (uint32_t i = 0; i < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry); i++) {
        dir_table->table[i] = empty_entry;
    }

    uint32_t empty_cluster_idx = find_empty_cluster(&fat32_driver_state.fat_table);
    if (empty_cluster_idx == (uint32_t) -1)
        return;

    init_directory_entry(&dir_table->table[0], name, "\0\0\0", ATTR_SUBDIRECTORY, empty_cluster_idx, 0);

    char* parent_name = "";
    if (parent_dir_cluster == ROOT_CLUSTER_NUMBER) {
        parent_name = "root\0\0\0\0";
    } else {
        struct ClusterBuffer c;
        read_clusters(&c, parent_dir_cluster, 1);
        memcpy(parent_name, &c, sizeof(dir_table->table[0].name));
    }
    
    init_directory_entry(&dir_table->table[1], parent_name, "\0\0\0", ATTR_SUBDIRECTORY, parent_dir_cluster, 0);
}

bool is_empty_storage(void) {
    struct BlockBuffer boot_sector;
    read_blocks(boot_sector.buf, BOOT_SECTOR, 1);
    return memcmp(boot_sector.buf, fs_signature, BLOCK_SIZE) != 0;
}

void create_fat32(void) {
    struct FAT32FileAllocationTable fat_table = {
        .cluster_map = { CLUSTER_0_VALUE, CLUSTER_1_VALUE, FAT32_FAT_EMPTY_ENTRY,
                         [3 ... CLUSTER_MAP_SIZE - 1] = FAT32_FAT_EMPTY_ENTRY }};
    
    fat32_driver_state.fat_table = fat_table;
    struct FAT32DirectoryTable root_directory_table;
    init_directory_table(&root_directory_table, "root\0\0\0\0", ROOT_CLUSTER_NUMBER);

    fat_table.cluster_map[2] = FAT32_FAT_END_OF_FILE;
    fat32_driver_state.fat_table = fat_table;
    fat32_driver_state.dir_table_buf = root_directory_table;

    write_clusters(fs_signature, BOOT_SECTOR, 1);
    write_clusters(&fat_table, FAT_CLUSTER_NUMBER, 1);
    write_clusters(&root_directory_table, ROOT_CLUSTER_NUMBER, 1);
}

void initialize_filesystem_fat32(void) {
    if (is_empty_storage()) {
        create_fat32();
    } else {
        read_clusters(&fat32_driver_state.fat_table, FAT_CLUSTER_NUMBER, 1);
    }
}

void write_clusters(const void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    write_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
}

void read_clusters(void *ptr, uint32_t cluster_number, uint8_t cluster_count) {
    read_blocks(ptr, cluster_to_lba(cluster_number), cluster_count * CLUSTER_BLOCK_COUNT);
}

int8_t read_directory(struct FAT32DriverRequest request) {
    // If buffer size is not enough to read the folder
    if (request.buffer_size < sizeof(struct FAT32DirectoryTable))
        return -1;

    struct FAT32DirectoryTable parent_directory_table;
    read_clusters(&parent_directory_table, request.parent_cluster_number, 1);
    
    // If parent entry is not a folder
    if (parent_directory_table.table[0].attribute != ATTR_SUBDIRECTORY) {
        return -1;
    }

    // Find requested entry
    uint32_t idx = 0;
    while (idx < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) &&
        memcmp(parent_directory_table.table[idx].name, request.name, sizeof(request.name)) != 0) {
        
        idx++;
    }
    // If requested entry not found
    if (idx == CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) {
        return 2;
    }
    // If requested entry is a file
    struct FAT32DirectoryEntry dir_entry = parent_directory_table.table[idx];
    if (dir_entry.attribute != ATTR_SUBDIRECTORY) {
        return 1;
    }
    
    uint32_t cluster_number = (dir_entry.cluster_high << 16) | dir_entry.cluster_low;
    read_clusters(request.buf, cluster_number, 1);
    
    return 0;
}

int8_t read(struct FAT32DriverRequest request) {
    struct FAT32DirectoryTable parent_directory_table;
    read_clusters(&parent_directory_table, request.parent_cluster_number, 1);
    
    if (parent_directory_table.table[0].attribute != ATTR_SUBDIRECTORY) {
        return -1;
    }

    uint32_t idx = 0;
    while (idx < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) &&
        !(memcmp(parent_directory_table.table[idx].name, request.name, sizeof(request.name)) == 0 &&
          memcmp(parent_directory_table.table[idx].ext, request.ext, sizeof(request.ext)) == 0)) {
        
        idx++;
    }

    if (idx == CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) {
        return 2;
    }

    struct FAT32DirectoryEntry dir_entry = parent_directory_table.table[idx];
    if (dir_entry.attribute == ATTR_SUBDIRECTORY) {
        return 1;
    }
    if (request.buffer_size < dir_entry.filesize)
        return -1;

    uint32_t cluster_number = (dir_entry.cluster_high << 16) | dir_entry.cluster_low;
    struct ClusterBuffer* ptr = request.buf;

    // Read all partitions of the file
    while (cluster_number != FAT32_FAT_END_OF_FILE) {
        read_clusters(ptr, cluster_number, 1);
        cluster_number = fat32_driver_state.fat_table.cluster_map[cluster_number];
        ptr++;
    }
    
    return 0;
}

int8_t write(struct FAT32DriverRequest request) {
    struct FAT32DirectoryTable parent_directory_table;
    read_clusters(&parent_directory_table, request.parent_cluster_number, 1);
    
    if (parent_directory_table.table[0].attribute != ATTR_SUBDIRECTORY) {
        return 2;
    }

    // Check if there's already an entry with the same name and ext (in the parent folder)
    bool found = false;
    uint32_t idx = 0, empty_entry_idx = -1, number_of_empty_entry = 0;
    while (idx < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) &&
        !(memcmp(parent_directory_table.table[idx].name, request.name, sizeof(request.name)) == 0 &&
          memcmp(parent_directory_table.table[idx].ext, request.ext, sizeof(request.ext)) == 0)) {
        
        if (parent_directory_table.table[idx].user_attribute != UATTR_NOT_EMPTY) {
            number_of_empty_entry++;
            if (!found) empty_entry_idx = idx;
            found = true;
        }
        idx++;
    }
    // If found an entry with the same name and ext
    if (idx < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) {
        return 1;
    }
    // If parent folder is full
    if (number_of_empty_entry == 0) {
        return -1;
    }

    // If write request is a folder
    if (request.buffer_size == 0) {
        uint32_t empty_cluster_idx = find_empty_cluster(&fat32_driver_state.fat_table);
        if (empty_cluster_idx == (uint32_t) -1)
            return -1;
        
        fat32_driver_state.fat_table.cluster_map[empty_cluster_idx] = FAT32_FAT_END_OF_FILE;
        init_directory_table(&fat32_driver_state.dir_table_buf, request.name, request.parent_cluster_number);
        init_directory_entry(&parent_directory_table.table[empty_entry_idx], request.name, request.ext, ATTR_SUBDIRECTORY, empty_cluster_idx, request.buffer_size);
        
        write_clusters(fat32_driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
        write_clusters(fat32_driver_state.dir_table_buf.table, empty_cluster_idx, 1);
        write_clusters(parent_directory_table.table, request.parent_cluster_number, 1);
    }
    // If write request is a file
    else {
        uint32_t number_of_partition = (request.buffer_size / CLUSTER_SIZE) + ((request.buffer_size % CLUSTER_SIZE > 0)? 1 : 0);
        if (!enough_empty_cluster(&fat32_driver_state.fat_table, number_of_partition)) {
            return -1;
        }

        uint32_t empty_cluster_idx = find_empty_cluster(&fat32_driver_state.fat_table);
        if (empty_cluster_idx == (uint32_t) -1)
            return -1;

        init_directory_entry(&parent_directory_table.table[empty_entry_idx], request.name, request.ext, 0, empty_cluster_idx, request.buffer_size);
        write_clusters(parent_directory_table.table, request.parent_cluster_number, 1);

        // Write each file partition
        uint32_t partitions_count = 0;
        while (partitions_count < number_of_partition - 1) {
            idx = empty_cluster_idx;
            empty_cluster_idx++;
            while (fat32_driver_state.fat_table.cluster_map[empty_cluster_idx] != FAT32_FAT_EMPTY_ENTRY)
                empty_cluster_idx++;

            fat32_driver_state.fat_table.cluster_map[idx] = empty_cluster_idx;
            write_clusters(request.buf + CLUSTER_SIZE*partitions_count, idx, 1);
            partitions_count++;
        }
        fat32_driver_state.fat_table.cluster_map[empty_cluster_idx] = FAT32_FAT_END_OF_FILE;
        write_clusters(request.buf + CLUSTER_SIZE*partitions_count, empty_cluster_idx, 1);
        write_clusters(fat32_driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);
    }

    return 0;
}

int8_t Delete(struct FAT32DriverRequest request) {
    struct FAT32DirectoryTable parent_directory_table;
    read_clusters(&parent_directory_table, request.parent_cluster_number, 1);
    
    if (parent_directory_table.table[0].attribute != ATTR_SUBDIRECTORY) {
        return -1;
    }

    uint32_t idx = 0;
    while (idx < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) &&
        !(memcmp(parent_directory_table.table[idx].name, request.name, sizeof(request.name)) == 0 &&
          memcmp(parent_directory_table.table[idx].ext, request.ext, sizeof(request.ext)) == 0)) {
        
        idx++;
    }

    if (idx == CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) {
        return 1;
    }

    struct FAT32DirectoryEntry dir_entry = parent_directory_table.table[idx];
    struct ClusterBuffer empty_cluster = {0};
    uint32_t deleted_cluster = (dir_entry.cluster_high << 16) | dir_entry.cluster_low;

    if (dir_entry.attribute == ATTR_SUBDIRECTORY) {
        struct FAT32DirectoryTable folder_directory_table;
        read_clusters(&folder_directory_table, dir_entry.cluster_low, 1);

        uint32_t idx2 = 2;
        while (idx2 < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry) &&
            folder_directory_table.table[idx2].user_attribute != UATTR_NOT_EMPTY) {
            
            idx2++;
        }
        // If the folder is not empty
        if (idx2 < CLUSTER_SIZE / sizeof(struct FAT32DirectoryEntry)) {
            return 2;
        }
    }
    else {
        uint32_t next_deleted_cluster;
        while (fat32_driver_state.fat_table.cluster_map[deleted_cluster] != FAT32_FAT_END_OF_FILE) {
            next_deleted_cluster = fat32_driver_state.fat_table.cluster_map[deleted_cluster];
            fat32_driver_state.fat_table.cluster_map[deleted_cluster] = FAT32_FAT_EMPTY_ENTRY;
            write_clusters(&empty_cluster, deleted_cluster, 1);
            deleted_cluster = next_deleted_cluster;
        }
    }

    memset(&parent_directory_table.table[idx], '\0' , sizeof(dir_entry));
    write_clusters(&empty_cluster, deleted_cluster, 1);
    fat32_driver_state.fat_table.cluster_map[deleted_cluster] = FAT32_FAT_EMPTY_ENTRY;
    write_clusters(parent_directory_table.table, request.parent_cluster_number, 1);
    write_clusters(fat32_driver_state.fat_table.cluster_map, FAT_CLUSTER_NUMBER, 1);

    return 0;
}