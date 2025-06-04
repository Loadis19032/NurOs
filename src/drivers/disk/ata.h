#ifndef ATA_H
#define ATA_H

#include "../../lib/stdlib/stdint.h"

// ATA PIO Ports (Primary Channel)
#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE_SEL   0x1F6
#define ATA_STATUS      0x1F7
#define ATA_COMMAND     0x1F7

// ATA Status Register Flags
#define ATA_SR_BSY      0x80    // Busy
#define ATA_SR_DRDY     0x40    // Drive Ready
#define ATA_SR_DF       0x20    // Drive Write Fault
#define ATA_SR_DRQ      0x08    // Data Request
#define ATA_SR_ERR      0x01    // Error

// ATA Commands
#define ATA_CMD_READ    0x20    // Read Sectors
#define ATA_CMD_WRITE   0x30    // Write Sectors
#define ATA_CMD_IDENTIFY 0xEC   // Identify Drive

// LBA (Logical Block Addressing) Modes
#define ATA_LBA_MASTER  0xE0    // Master Drive
#define ATA_LBA_SLAVE   0xF0    // Slave Drive

// Function Prototypes
void ata_wait_ready();
void ata_wait_drq();
int ata_identify(uint16_t *buffer);
void ata_read_sectors(uint32_t lba, uint8_t count, uint16_t *buffer);
void ata_write_sectors(uint32_t lba, uint8_t count, uint16_t *buffer);

#endif // ATA_H