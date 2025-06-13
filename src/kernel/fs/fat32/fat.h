#ifndef FAT_H
#define FAT_H

#include <stdint.h>

// FAT32 BIOS Parameter Block (BPB) and Extended Boot Record (EBR)
typedef struct {
    uint8_t jump[3];
    char oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    
    // Extended Boot Record (EBR) for FAT32
    uint32_t fat_size_32;
    uint16_t flags;
    uint16_t version;
    uint32_t root_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
} __attribute__((packed)) fat_bpb_t;

// FAT32 Directory Entry
typedef struct {
    char name[8];
    char ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t access_date;
    uint16_t cluster_high;
    uint16_t modification_time;
    uint16_t modification_date;
    uint16_t cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat_dir_entry_t;

// File handle
typedef struct {
    uint32_t start_cluster;
    uint32_t current_cluster;
    uint32_t file_size;
    uint32_t position;
    uint32_t current_sector_in_cluster;
    uint32_t cluster_index;
} fat_file_t;

// Initialize FAT32 filesystem
int fat_init(uint32_t lba);

// Open a file
int fat_open(fat_file_t *file, const char *path);

// Read data from file
uint32_t fat_read(fat_file_t *file, void *buffer, uint32_t size);

// Close file (just resets the structure)
void fat_close(fat_file_t *file);

#endif // FAT_H