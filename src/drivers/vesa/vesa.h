#ifndef VESA_H
#define VESA_H

#include "../../lib/stdlib/stdint.h"

// VESA Mode Information Structure
typedef struct {
    uint16_t attributes;
    uint8_t window_a, window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a, segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch; // bytes per scanline
    
    uint16_t width, height;
    uint8_t w_char, y_char, planes, bpp, banks;
    uint8_t memory_model, bank_size, image_pages;
    uint8_t reserved0;
    
    uint8_t red_mask, red_position;
    uint8_t green_mask, green_position;
    uint8_t blue_mask, blue_position;
    uint8_t rsv_mask, rsv_position;
    uint8_t directcolor_attributes;
    
    uint32_t framebuffer; // physical address of the linear frame buffer
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
} __attribute__((packed)) vesa_mode_info_t;

// VESA Controller Information Structure
typedef struct {
    char signature[4];
    uint16_t version;
    uint32_t oem_string_ptr;
    uint32_t capabilities;
    uint32_t video_mode_ptr;
    uint16_t total_memory;
    uint16_t oem_software_rev;
    uint32_t oem_vendor_name_ptr;
    uint32_t oem_product_name_ptr;
    uint32_t oem_product_rev_ptr;
    uint8_t reserved[222];
    uint8_t oem_data[256];
} __attribute__((packed)) vesa_info_t;

// Function prototypes
int vesa_init();
int vesa_set_mode(uint16_t mode);
vesa_mode_info_t* vesa_get_mode_info();
void* vesa_get_framebuffer();
uint32_t vesa_get_width();
uint32_t vesa_get_height();
uint32_t vesa_get_bpp();
uint32_t vesa_get_pitch();

#endif // VESA_H