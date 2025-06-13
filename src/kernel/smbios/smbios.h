#ifndef SMBIOS_H
#define SMBIOS_H

#include "../../lib/stdlib/stdint.h"

#define SMBIOS_TYPE_PHYSICAL_MEMORY_ARRAY 16
#define SMBIOS_TYPE_MEMORY_DEVICE 17
#define SMBIOS_TYPE_END_OF_TABLE 127

typedef struct {
    uint8_t type;
    uint8_t length;
    uint16_t handle;
} __attribute__((packed)) smbios_header_t;

typedef struct {
    char anchor[4];           // "_SM_"
    uint8_t checksum;
    uint8_t length;
    uint8_t major_version;
    uint8_t minor_version;
    uint16_t max_struct_size;
    uint8_t revision;
    char formatted_area[5];
    char intermediate_anchor[5]; // "_DMI_"
    uint8_t intermediate_checksum;
    uint16_t table_length;
    uint32_t table_address;
    uint16_t number_of_structures;
    uint8_t bcd_revision;
} __attribute__((packed)) smbios_entry_point_t;

typedef struct {
    smbios_header_t header;
    uint8_t location;
    uint8_t use;
    uint8_t memory_error_correction;
    uint32_t maximum_capacity;   
    uint16_t memory_error_info_handle;
    uint16_t number_of_memory_devices;
    uint64_t extended_maximum_capacity; 
} __attribute__((packed)) smbios_physical_memory_array_t;

typedef struct {
    smbios_header_t header;
    uint16_t physical_memory_array_handle;
    uint16_t memory_error_info_handle;
    uint16_t total_width;
    uint16_t data_width;
    uint16_t size;              
    uint8_t form_factor;
    uint8_t device_set;
    uint8_t device_locator;     
    uint8_t bank_locator;       
    uint8_t memory_type;
    uint16_t type_detail;
    uint16_t speed;
    uint8_t manufacturer;       
    uint8_t serial_number;      
    uint8_t asset_tag;          
    uint8_t part_number;        
    uint8_t attributes;
    uint32_t extended_size;     
    uint16_t configured_speed;
    uint16_t minimum_voltage;
    uint16_t maximum_voltage;
    uint16_t configured_voltage;
} __attribute__((packed)) smbios_memory_device_t;

int smbios_init(void);
uint64_t smbios_get_total_memory_kb(void);
int smbios_parse_memory_devices(void);
int smbios_parse_physical_memory_arrays(void);
uint64_t cmos_get_memory_kb(void);
uint64_t detect_memory_fallback(void);

const char* smbios_get_string(const uint8_t* structure, uint8_t index);
uint8_t* smbios_find_entry_point(void);
int smbios_validate_checksum(const void* data, int length);

void smbios_demo(void);

extern uint8_t* smbios_table_data;
extern uint32_t smbios_table_size;
extern uint64_t total_memory_kb;

#endif // SMBIOS_H