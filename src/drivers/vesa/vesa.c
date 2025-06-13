#include "vesa.h"
#include "../../lib/util.h"
#include "../../lib/stdlib/string.h"
#include "../../kernel/mem/malloc.h"

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA  0x01CF

// VBE Functions
#define VBE_FUNCTION_GET_CONTROLLER_INFO 0x4F00
#define VBE_FUNCTION_GET_MODE_INFO       0x4F01
#define VBE_FUNCTION_SET_MODE            0x4F02

// VBE Return Status
#define VBE_RETURN_STATUS_SUPPORTED 0x004F

// QEMU-specific VBE modes (примерные)
#define QEMU_VBE_640x480x32  0x4111
#define QEMU_VBE_800x600x32  0x4112
#define QEMU_VBE_1024x768x32 0x4115
#define QEMU_VBE_1280x1024x32 0x4118

static vesa_info_t vesa_info __attribute__((aligned(16)));
static vesa_mode_info_t vesa_mode_info __attribute__((aligned(16)));
static uint32_t* framebuffer = NULL;

static void vesa_write_index(uint16_t index) {
    outPortW(VBE_DISPI_IOPORT_INDEX, index);
}

static uint16_t vesa_read_data() {
    return inPortW(VBE_DISPI_IOPORT_DATA);
}

static void vesa_write_data(uint16_t value) {
    outPortW(VBE_DISPI_IOPORT_DATA, value);
}

int vesa_init() {
    // 1. Получаем информацию о VBE контроллере
    memset(&vesa_info, 0, sizeof(vesa_info));
    memcpy(vesa_info.signature, "VBE2", 4);

    vesa_write_index(VBE_FUNCTION_GET_CONTROLLER_INFO);
    vesa_write_data(0); // Segment = 0 (мы используем плоскую модель памяти)
    vesa_write_index(VBE_FUNCTION_GET_CONTROLLER_INFO + 1);
    vesa_write_data((uint16_t)((uint32_t)&vesa_info)); // Offset

    // Проверяем статус
    if (vesa_read_data() != VBE_RETURN_STATUS_SUPPORTED) {
        return 1;
    }

    // Проверяем сигнатуру
    if (memcmp(vesa_info.signature, "VESA", 4) != 0) {
        return 1;
    }

    return 0;
}

int vesa_set_mode(uint16_t mode) {
    // 1. Получаем информацию о режиме
    memset(&vesa_mode_info, 0, sizeof(vesa_mode_info));

    vesa_write_index(VBE_FUNCTION_GET_MODE_INFO);
    vesa_write_data(0); // Segment = 0
    vesa_write_index(VBE_FUNCTION_GET_MODE_INFO + 1);
    vesa_write_data((uint16_t)((uint32_t)&vesa_mode_info)); // Offset
    vesa_write_index(VBE_FUNCTION_GET_MODE_INFO + 2);
    vesa_write_data(mode);

    if (vesa_read_data() != VBE_RETURN_STATUS_SUPPORTED) {
        return 1;
    }

    // 2. Устанавливаем режим с LFB
    vesa_write_index(VBE_FUNCTION_SET_MODE);
    vesa_write_data(mode | 0x4000); // LFB flag

    if (vesa_read_data() != VBE_RETURN_STATUS_SUPPORTED) {
        return 1;
    }

    framebuffer = (uint32_t*)vesa_mode_info.framebuffer;
    return 0;
}

// Остальные функции остаются без изменений
vesa_mode_info_t* vesa_get_mode_info() { return &vesa_mode_info; }
void* vesa_get_framebuffer() { return framebuffer; }
uint32_t vesa_get_width() { return vesa_mode_info.width; }
uint32_t vesa_get_height() { return vesa_mode_info.height; }
uint32_t vesa_get_bpp() { return vesa_mode_info.bpp; }
uint32_t vesa_get_pitch() { return vesa_mode_info.pitch; }