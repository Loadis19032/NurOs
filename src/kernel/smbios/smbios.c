#include "smbios.h"
#include "../../lib/stdlib/string.h"
#include "../../lib/util.h"
#include "../../lib/stdlib/stdio.h"
#include "../../drivers/vga/vga.h"

uint8_t* smbios_table_data = (uint8_t*)0;
uint32_t smbios_table_size = 0;
uint64_t total_memory_kb = 0;

uint8_t* smbios_find_entry_point(void) {
    uint8_t* mem = (uint8_t*)0xF0000;
    
    for (uint32_t i = 0; i < 0x10000; i += 16) {
        if (memcmp(mem + i, "_SM_", 4) == 0) {
            smbios_entry_point_t* ep = (smbios_entry_point_t*)(mem + i);
            if (smbios_validate_checksum(ep, ep->length)) {
                return (uint8_t*)ep;
            }
        }
    }
    
    return (uint8_t*)0;
}

int smbios_validate_checksum(const void* data, int length) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint8_t sum = 0;
    
    for (int i = 0; i < length; i++) {
        sum += bytes[i];
    }
    
    return sum == 0;
}

const char* smbios_get_string(const uint8_t* structure, uint8_t index) {
    if (index == 0) return "";
    
    const uint8_t* strings = structure + ((smbios_header_t*)structure)->length;
    uint8_t current_index = 1;
    
    while (*strings != 0) {
        if (current_index == index) {
            return (const char*)strings;
        }
        
        while (*strings != 0) strings++;
        strings++;
        current_index++;
    }
    
    return "";
}

int smbios_init(void) {
    uint8_t* entry_point = smbios_find_entry_point();
    if (entry_point == (uint8_t*)0) {
        print("SMBIOS Entry Point not found\n");
        return -1;
    }
    
    smbios_entry_point_t* ep = (smbios_entry_point_t*)entry_point;
    
    if (!smbios_validate_checksum(&ep->intermediate_anchor[0], 15)) {
        print("wrong control sum SMBIOS\n");
        return -1;
    }
    
    smbios_table_data = (uint8_t*)(uint32_t)ep->table_address;
    smbios_table_size = ep->table_length;
    
    printf("SMBIOS finded: version %d.%d, table of adress 0x%X, size %d byte\n",
           ep->major_version, ep->minor_version, ep->table_address, ep->table_length);
    
    return 0;
}

uint64_t cmos_get_memory_kb(void) {
    uint32_t low_mem = 0;
    uint32_t high_mem = 0;
    
    outPortB(0x70, 0x15);
    low_mem = inPortB(0x71);
    outPortB(0x70, 0x16);
    low_mem |= (inPortB(0x71) << 8);
    
    outPortB(0x70, 0x17);
    high_mem = inPortB(0x71);
    outPortB(0x70, 0x18);
    high_mem |= (inPortB(0x71) << 8);
    
    outPortB(0x70, 0x30);
    uint32_t alt_high = inPortB(0x71);
    outPortB(0x70, 0x31);
    alt_high |= (inPortB(0x71) << 8);
    
    if (alt_high > high_mem) {
        high_mem = alt_high;
    }
    
    uint64_t total = (uint64_t)low_mem + (uint64_t)high_mem + 1024;
    
    printf("CMOS memory: base=%d KB, extended=%d KB, all=%d KB\n", 
           low_mem, high_mem, total);
    
    return total;
}

uint64_t detect_memory_fallback(void) {
    uint64_t cmos_memory = cmos_get_memory_kb();
    if (cmos_memory > 0) {
        printf("finded is CMOS: %d KB\n", cmos_memory);
        return cmos_memory;
    }
    
    uint32_t* test_addr;
    uint32_t mb_count = 0;
    uint32_t test_pattern = 0x12345678;
    uint32_t saved_value;
    
    for (mb_count = 1; mb_count < 4096; mb_count++) {
        test_addr = (uint32_t*)(mb_count * 1024 * 1024);

        saved_value = *test_addr;
        
        *test_addr = test_pattern;
        
        if (*test_addr != test_pattern) {
            break;
        }
        
        *test_addr = saved_value;
    }
    
    uint64_t detected_kb = (uint64_t)mb_count * 1024;
    
    return detected_kb;
}

int smbios_parse_physical_memory_arrays(void) {
    if (smbios_table_data == (uint8_t*)0) {
        print("SMBIOS not init\n");
        return -1;
    }
    
    uint8_t* current = smbios_table_data;
    uint8_t* end = smbios_table_data + smbios_table_size;
    int array_count = 0;
    uint64_t total_from_arrays = 0;
    
    while (current < end) {
        smbios_header_t* header = (smbios_header_t*)current;
        
        if (current + header->length > end) {
            break;
        }
        
        if (header->type == SMBIOS_TYPE_END_OF_TABLE) {
            break;
        }
        
        if (header->type == SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY) {
            array_count++;
            
            if (header->length >= 15) {
                smbios_physical_memory_array_t* mem_array = (smbios_physical_memory_array_t*)current;
                
                if (mem_array->maximum_capacity != 0x80000000) {
                    total_from_arrays += mem_array->maximum_capacity;
                } else if (header->length >= sizeof(smbios_physical_memory_array_t)) {
                    uint64_t extended_kb = mem_array->extended_maximum_capacity / 1024;
                    total_from_arrays += extended_kb;
                }
            }
        }
        
        current += header->length;
        
        while (current < end - 1 && !(current[0] == 0 && current[1] == 0)) {
            current++;
        }
        if (current < end - 1) {
            current += 2;
        }
    }
    
    if (total_memory_kb == 0 && total_from_arrays > 0) {
        total_memory_kb = total_from_arrays;
    }
    
    return 0;
}

// Парсинг устройств памяти
int smbios_parse_memory_devices(void) {
    if (smbios_table_data == (uint8_t*)0) {
        print("SMBIOS not init\n");
        return -1;
    }
    
    total_memory_kb = 0;
    uint8_t* current = smbios_table_data;
    uint8_t* end = smbios_table_data + smbios_table_size;
    int memory_device_count = 0;
    int total_structures = 0;
    
    while (current < end) {
        smbios_header_t* header = (smbios_header_t*)current;
        
        // Проверяем, не вышли ли за границы
        if (current + header->length > end) {
            break;
        }
        
        total_structures++;
        
        if (header->type == SMBIOS_TYPE_END_OF_TABLE) {
            break;
        }
        
        if (header->type == SMBIOS_TYPE_MEMORY_DEVICE) {
            memory_device_count++;
            
            if (header->length >= 21) {
                smbios_memory_device_t* mem_dev = (smbios_memory_device_t*)current;
                uint64_t size_kb = 0;
                
                if (mem_dev->size != 0x7FFF && mem_dev->size != 0xFFFF && mem_dev->size != 0) {
                    if (mem_dev->size & 0x8000) {
                        size_kb = mem_dev->size & 0x7FFF;
                    } else {
                        size_kb = (uint64_t)mem_dev->size * 1024;
                    }
                }
                
                if (header->length >= sizeof(smbios_memory_device_t)) {
                    if (mem_dev->extended_size != 0) {
                        size_kb = (uint64_t)mem_dev->extended_size * 1024;
                    }
                }
                
                if (size_kb > 0) {
                    total_memory_kb += size_kb;
                }
            }
        }
        
        current += header->length;
        
        while (current < end - 1 && !(current[0] == 0 && current[1] == 0)) {
            current++;
        }
        if (current < end - 1) {
            current += 2;
        }
    }
    
    return 0;
}

uint64_t smbios_get_total_memory_kb(void) {
    return total_memory_kb;
}

void smbios_demo(void) {
    print("=== detector memory ===\n");
    print("init SMBIOS...\n");
    
    uint64_t final_memory_kb = 0;
    
    if (smbios_init() == 0) {
        print("\n=== metode 1: device memory SMBIOS (Type 17) ===\n");
        smbios_parse_memory_devices();
        
        print("\n=== metode 2: physical memory array SMBIOS (Type 16) ===\n");
        smbios_parse_physical_memory_arrays();
        
        final_memory_kb = smbios_get_total_memory_kb();
        
        if (final_memory_kb > 0) {
            printf("SMBIOS success memory: %d KB\n", final_memory_kb);
        } else {
            print("SMBIOS not memory, used metode...\n");
        }
    } else {
        print("SMBIOS not, used metode\n");
    }
    
    if (final_memory_kb == 0) {
        printf("\n=== metode 3: CMOS and scaning memory ===\n");
        final_memory_kb = detect_memory_fallback();
        
        total_memory_kb = final_memory_kb;
    }
    
    uint64_t total_mb = final_memory_kb / 1024;
    uint64_t total_gb = total_mb / 1024;
    
    print("\n=== RESULT ===\n");
    if (final_memory_kb > 0) {
        printf("Amount size RAM: %d KB\n", final_memory_kb);
        printf("Amount size RAM: %d MB\n", total_mb);
        printf("Amount size RAM: %d GB\n", total_gb);
    } else {
        print("RAM erorr\n");
    }
}