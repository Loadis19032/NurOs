#include "fat.h"
#include "../../../drivers/disk/ata.h"
#include "../../../lib/stdlib/string.h"
#include "../../../lib/stdlib/stdint.h"
#include "../../mem/malloc.h"
#include "../../../lib/util.h"

static fat_bpb_t bpb;
static uint32_t fat_lba;
static uint32_t data_lba;
static uint32_t sectors_per_fat;
static uint32_t root_dir_sectors;
static uint32_t cluster_size;

// Read FAT32 BPB from disk
static bool read_bpb(uint32_t lba) {
    uint16_t buffer[256];
    ata_read_sectors(lba, 1, buffer);
    memcpy(&bpb, buffer, sizeof(fat_bpb_t));
    
    // Check FS type (should be "FAT32   ")
    if (memcmp(bpb.fs_type, "FAT32   ", 8) != 0) {
        return false;
    }
    
    // Calculate important values
    sectors_per_fat = bpb.fat_size_32;
    root_dir_sectors = ((bpb.root_entries * 32) + (bpb.bytes_per_sector - 1)) / bpb.bytes_per_sector;
    fat_lba = lba + bpb.reserved_sectors;
    data_lba = fat_lba + (bpb.fat_count * sectors_per_fat) + root_dir_sectors;
    cluster_size = bpb.sectors_per_cluster * bpb.bytes_per_sector;
    
    return true;
}

// Get next cluster in FAT chain
static uint32_t get_next_cluster(uint32_t cluster) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fat_lba + (fat_offset / bpb.bytes_per_sector);
    uint32_t ent_offset = fat_offset % bpb.bytes_per_sector;
    
    uint16_t buffer[256];
    ata_read_sectors(fat_sector, 1, buffer);
    
    uint32_t next_cluster = *((uint32_t*)&buffer[ent_offset/2]);
    next_cluster &= 0x0FFFFFFF; // FAT32 uses 28 bits
    
    return (next_cluster >= 0x0FFFFFF8) ? 0 : next_cluster;
}

// Find file in directory
static bool find_file_in_dir(uint32_t cluster, const char *name, fat_dir_entry_t *entry) {
    uint16_t buffer[256];
    uint32_t sector = (cluster == 0) ? 
        (fat_lba + (bpb.fat_count * sectors_per_fat)) : 
        (data_lba + (cluster - 2) * bpb.sectors_per_cluster);
    
    char short_name[12];
    memset(short_name, ' ', 11);
    short_name[11] = '\0';
    
    // Convert name to 8.3 format
    const char *dot = strchr(name, '.');
    if (dot) {
        int name_len = dot - name;
        if (name_len > 8) name_len = 8;
        memcpy(short_name, name, name_len);
        
        int ext_len = strlen(dot + 1);
        if (ext_len > 3) ext_len = 3;
        memcpy(short_name + 8, dot + 1, ext_len);
    } else {
        int name_len = strlen(name);
        if (name_len > 8) name_len = 8;
        memcpy(short_name, name, name_len);
    }
    
    // Search directory entries
    for (uint32_t s = 0; s < bpb.sectors_per_cluster; s++) {
        ata_read_sectors(sector + s, 1, buffer);
        fat_dir_entry_t *dir = (fat_dir_entry_t *)buffer;
        
        for (uint32_t i = 0; i < (bpb.bytes_per_sector / sizeof(fat_dir_entry_t)); i++) {
            if (dir[i].name[0] == 0x00) return false; // End of directory
            if (dir[i].name[0] == 0xE5) continue;     // Deleted entry
            
            // Compare names
            char entry_name[12];
            memcpy(entry_name, dir[i].name, 8);
            memcpy(entry_name + 8, dir[i].ext, 3);
            
            if (memcmp(entry_name, short_name, 11) == 0) {
                memcpy(entry, &dir[i], sizeof(fat_dir_entry_t));
                return true;
            }
        }
    }
    
    return false;
}

// Initialize FAT32 filesystem
int fat_init(uint32_t lba) {
    if (!read_bpb(lba)) {
        return -1;
    }
    return 0;
}

// Open a file
int fat_open(fat_file_t *file, const char *path) {
    fat_dir_entry_t entry;
    
    // Start from root directory
    if (!find_file_in_dir(bpb.root_cluster, path, &entry)) {
        return -1;
    }
    
    // Fill file structure
    file->start_cluster = (entry.cluster_high << 16) | entry.cluster_low;
    file->current_cluster = file->start_cluster;
    file->file_size = entry.file_size;
    file->position = 0;
    file->current_sector_in_cluster = 0;
    file->cluster_index = 0;
    
    return 0;
}

// Read data from file
uint32_t fat_read(fat_file_t *file, void *buffer, uint32_t size) {
    if (file->position >= file->file_size) return 0;
    
    uint32_t remaining = file->file_size - file->position;
    if (size > remaining) size = remaining;
    
    uint32_t bytes_read = 0;
    uint8_t *buf = (uint8_t*)buffer;
    
    while (bytes_read < size) {
        uint32_t cluster_offset = file->position % cluster_size;
        uint32_t remaining_in_cluster = cluster_size - cluster_offset;
        uint32_t to_read = (size - bytes_read < remaining_in_cluster) ? 
            size - bytes_read : remaining_in_cluster;
        
        // Calculate sector address
        uint32_t sector = data_lba + (file->current_cluster - 2) * bpb.sectors_per_cluster;
        sector += file->current_sector_in_cluster;
        
        // Read sector
        uint16_t sector_buffer[256];
        ata_read_sectors(sector, 1, sector_buffer);
        
        // Copy data
        uint32_t offset_in_sector = cluster_offset % bpb.bytes_per_sector;
        uint32_t to_copy = bpb.bytes_per_sector - offset_in_sector;
        if (to_copy > to_read) to_copy = to_read;
        
        memcpy(buf + bytes_read, (uint8_t*)sector_buffer + offset_in_sector, to_copy);
        bytes_read += to_copy;
        file->position += to_copy;
        
        // Move to next sector/cluster if needed
        file->current_sector_in_cluster++;
        if (file->current_sector_in_cluster >= bpb.sectors_per_cluster) {
            file->current_sector_in_cluster = 0;
            file->current_cluster = get_next_cluster(file->current_cluster);
            if (file->current_cluster == 0) break; // End of chain
        }
    }
    
    return bytes_read;
}

// Close file (just resets the structure)
void fat_close(fat_file_t *file) {
    memset(file, 0, sizeof(fat_file_t));
}