#include "ata.h"
#include "../../lib/util.h"  // Предполагается, что у вас есть функции inb(), outb(), inw(), outw()

// Ожидание готовности диска
void ata_wait_ready() {
    while (inPortB(ATA_STATUS) & ATA_SR_BSY);  // Ждём, пока диск не перестанет быть занят
    while (!(inPortB(ATA_STATUS) & ATA_SR_DRDY));  // Ждём, пока диск будет готов
}

// Ожидание запроса данных (DRQ)
void ata_wait_drq() {
    while (!(inPortB(ATA_STATUS) & ATA_SR_DRQ));
}

// Идентификация диска (возвращает 0 при успехе)
int ata_identify(uint16_t *buffer) {
    ata_wait_ready();

    // Выбираем Master-диск
    outPortB(ATA_DRIVE_SEL, ATA_LBA_MASTER);
    ata_wait_ready();

    // Отправляем команду IDENTIFY
    outPortB(ATA_COMMAND, ATA_CMD_IDENTIFY);
    ata_wait_ready();

    // Если диск не существует, статус == 0
    if (inPortB(ATA_STATUS) == 0) {
        return -1;
    }

    ata_wait_drq();

    // Читаем 256 слов (512 байт) данных
    for (int i = 0; i < 256; i++) {
        buffer[i] = inPortW(ATA_DATA);
    }

    return 0;
}

// Чтение секторов (LBA 28-bit)
void ata_read_sectors(uint32_t lba, uint8_t count, uint16_t *buffer) {
    ata_wait_ready();

    // Устанавливаем параметры LBA
    outPortB(ATA_DRIVE_SEL, ATA_LBA_MASTER | ((lba >> 24) & 0x0F));
    outPortB(ATA_SECTOR_CNT, count);
    outPortB(ATA_LBA_LOW, (uint8_t)(lba & 0xFF));
    outPortB(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outPortB(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    // Отправляем команду READ
    outPortB(ATA_COMMAND, ATA_CMD_READ);
    ata_wait_ready();

    // Читаем данные
    for (int i = 0; i < 256 * count; i++) {
        buffer[i] = inPortW(ATA_DATA);
    }
}

// Запись секторов (LBA 28-bit)
void ata_write_sectors(uint32_t lba, uint8_t count, uint16_t *buffer) {
    ata_wait_ready();

    // Устанавливаем параметры LBA
    outPortB(ATA_DRIVE_SEL, ATA_LBA_MASTER | ((lba >> 24) & 0x0F));
    outPortB(ATA_SECTOR_CNT, count);
    outPortB(ATA_LBA_LOW, (uint8_t)(lba & 0xFF));
    outPortB(ATA_LBA_MID, (uint8_t)(lba >> 8));
    outPortB(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    // Отправляем команду WRITE
    outPortB(ATA_COMMAND, ATA_CMD_WRITE);
    ata_wait_drq();

    // Записываем данные
    for (int i = 0; i < 256 * count; i++) {
        outPortW(ATA_DATA, buffer[i]);
    }

    // Ожидаем завершения записи
    ata_wait_ready();
}