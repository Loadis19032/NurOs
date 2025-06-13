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
#include "apps/terminal.h"
#include "acpi/acpi.h"
//#include "../drivers/vesa/vesa.h"
#include "smbios/smbios.h"



//void main(uint32_t magic, struct multiboot_info* boot);
void kmain(uint32_t magic, struct multiboot_info* boot) {
    initGdt();
    print("Gdt init\n");
    initIdt();
    print("Idt init\n");
    initTimer();
    print("Timer init\n");
    initKeyboard();
    print("Keyboard driver init\n");
    init();

    print("init SMBIOS...\n");
    
    uint64_t final_memory_kb = 0;
    
    if (smbios_init() == 0) {
        smbios_parse_memory_devices();
        smbios_parse_physical_memory_arrays();
        
        final_memory_kb = smbios_get_total_memory_kb();
        
        if (final_memory_kb > 0) {
            printf("SMBIOS success, memory size: %d KB\n", final_memory_kb);
        }
    }
    
    if (final_memory_kb == 0) {
        final_memory_kb = detect_memory_fallback();
        total_memory_kb = final_memory_kb;
    }
    
    if (final_memory_kb > 0) {
        printf("Amount size RAM: %d KB\n", final_memory_kb);
    } 
    
    printf("mem upper: %x\n", boot->mem_upper * 1024);
    printf("mem upper custom: %x\n", final_memory_kb * 1024);


    uint32_t end = 0x200000;
    uint32_t physicalAllocStart = (end + 0xFFF) & ~0xFFF;
    initMemory((final_memory_kb * 1024), physicalAllocStart); //0x7EE0000 0xF0010000
    kmallocInit(0x1000);

    uint16_t identify_data[256];
    if (ata_identify(identify_data) == 0) {
        print("ATA identify failed!\n");
    } else {
        print("Disk detected\n");
    }

    print("Nur--> ");

    for (;;);
}
