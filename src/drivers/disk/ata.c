#include "ata.h"
#include "../../lib/util.h"

// Ожидание готовности диска
void ata_wait_ready() {
    while (inPortB(ATA_STATUS) & ATA_SR_BSY);
    while (!(inPortB(ATA_STATUS) & ATA_SR_DRDY));
}

void ata_wait_drq() {
    while (!(inPortB(ATA_STATUS) & ATA_SR_DRQ));
}

int ata_identify(uint16_t *buffer) {
    ata_wait_ready();

    outPortB(ATA_DRIVE_SEL, ATA_LBA_MASTER);
    ata_wait_ready();

    outPortB(ATA_COMMAND, ATA_CMD_IDENTIFY);
    ata_wait_ready();

    if (inPortB(ATA_STATUS) == 0) {
        return -1;
    }

    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        buffer[i] = inPortW(ATA_DATA);
    }

    return 0;
}

void ata_read_sectors(uint32_t lba, uint8_t count, uint16_t *buffer) {
    ata_wait_ready();

    outPortB(ATA_DRIVE_SEL, ATA_LBA_MASTER | ((lba >> 24) & 0x0F));
    outPortB(ATA_SECTOR_CNT, count);
    outPortB(ATA_LBA_LOW, (uint8_t)(lba & 0xFF));
    outPortB(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outPortB(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    outPortB(ATA_COMMAND, ATA_CMD_READ);
    ata_wait_ready();

    for (int i = 0; i < 256 * count; i++) {
        buffer[i] = inPortW(ATA_DATA);
    }
}

void ata_write_sectors(uint32_t lba, uint8_t count, uint16_t *buffer) {
    ata_wait_ready();

    outPortB(ATA_DRIVE_SEL, ATA_LBA_MASTER | ((lba >> 24) & 0x0F));
    outPortB(ATA_SECTOR_CNT, count);
    outPortB(ATA_LBA_LOW, (uint8_t)(lba & 0xFF));
    outPortB(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outPortB(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    outPortB(ATA_COMMAND, ATA_CMD_WRITE);
    ata_wait_drq();

    for (int i = 0; i < 256 * count; i++) {
        outPortW(ATA_DATA, buffer[i]);
    }

    ata_wait_ready();
}