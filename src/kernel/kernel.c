#include "../drivers/vga/vga.h"
#include "../lib/stdlib/stdint.h"
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "timer/timer.h"
#include "../lib/util.h"
#include "../lib/stdlib/stdio.h"
#include "../drivers/keyboard/keyboard.h"
#include "mem/malloc.h"
#include "fs/vfs/vfs.h"
#include "boot/multiboot.h"
#include "../lib/mem/mem.h"
#include "fs/fat32/fat.h"
#include "../drivers/disk/ata.h"

//void main(uint32_t magic, struct multiboot_info* boot);
void main(uint32_t magic, struct multiboot_info* boot) {
    initGdt();
    print("Gdt init\n");
    initIdt();
    print("Idt init\n");
    initTimer();
    print("Timer init\n");
    initKeyboard();
    print("Keyboard driver init\n");
    uint32_t mod1 = *(uint32_t*)(boot->mods_addr + 4);
    uint32_t physicalAllocStart = (mod1 + 0xFFF) & ~0xFFF;
    initMemory(boot->mem_upper * 1024, physicalAllocStart);
    print("mod1: ");
    printf("%p", mod1);
    print(", PhysicalAllocStart: ");
    printf("%p", physicalAllocStart);
    kmallocInit(0x1000); 
    print("\nInit memory: 0x1000\n");
    init();
    print("Init VFS\n");

    uint16_t identify_data[256];
    if (ata_identify(identify_data) == 0) {
        print("Disk finded!\n");
        uint16_t sector_buffer[256];
        ata_read_sectors(0, 1, sector_buffer);
    }

    print("NurOs-> ");

    //set_screen_color(0x6F);
    for(;;);
}
