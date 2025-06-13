#include "vga.h"
#include "stdint.h"

uint16_t column = 0;
uint16_t line = 0;
uint16_t* const vga = (uint16_t* const) 0xC00B8000;
const uint16_t defaultColor = (COLOR8_WHITE << 8)|(COLOR8_BLACK << 12);
uint16_t currentColor = defaultColor;

void SetScreenColor(uint8_t color) {
    uint8_t* video_memory = (uint8_t*)0xC00B8000;

    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        video_memory[i + 1] = color;
    }
}

const char* split(const char *str) {
    if (!str) return 0;  // Проверка на NULL

    // Ищем первый пробел
    while (*str != ' ' && *str != '\0') {
        str++;
    }

    // Если пробел не найден, возвращаем NULL
    if (*str == '\0') {
        return 0;
    }

    // Возвращаем указатель на символ после пробела
    return str + 1;
}

void ClearScreen(){
    line = 0;
    column = 0;
    currentColor = defaultColor;

    for (uint16_t y = 0; y < VGA_HEIGHT; y++){
        for (uint16_t x = 0; x < VGA_WIDTH; x++){
            vga[y * VGA_WIDTH + x] = ' ' | defaultColor;
        }
    }
}

void newLine(){
    if (line < VGA_HEIGHT - 1){
        line++;
        column = 0;
    }
    else{
      scrollUp();
      column = 0;
    }
}

void scrollUp() {
    // Сдвигаем строки вверх, начиная с y=1
    for (uint16_t y = 1; y < VGA_HEIGHT; y++) {
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            vga[(y - 1) * VGA_WIDTH + x] = vga[y * VGA_WIDTH + x];
        }
    }
    
    // Очищаем нижнюю строку (y=VGA_HEIGHT-1) пробелами с текущим цветом
    for (uint16_t x = 0; x < VGA_WIDTH; x++) {
        vga[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ' ' | currentColor;
    }
}

void scrollDown() {
    for (uint16_t y = VGA_HEIGHT - 1; y > 0; y--) {
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            vga[y * VGA_WIDTH + x] = vga[(y - 1) * VGA_WIDTH + x];
        }
    }
    for (uint16_t x = 0; x < VGA_WIDTH; x++) {
        vga[x] = ' ' | currentColor;
    }
}

void print(const char* s){
    while(*s){
        switch(*s){
            case '\n':
                newLine();
                break;
            case '\r':
                column = 0;
                break;
            case '\b':
                if(column == 8) {
                    vga[line * VGA_WIDTH + (++column)] = ' ' | currentColor;
                    break;
                }
                if (column == 0 && line != 0){
                    line--;
                    column = VGA_WIDTH;
                }
                vga[line * VGA_WIDTH + (--column)] = ' ' | currentColor;
                break;
            case '\t':
                if (column == VGA_WIDTH){
                    newLine();
                }
                uint16_t tabLen = 4 - (column % 4);
                while (tabLen != 0){
                    vga[line * VGA_WIDTH + (column++)] = ' ' | currentColor;
                    tabLen--;
                }
                break;
            default:
                if (column == VGA_WIDTH){
                    newLine();
                }

                vga[line * VGA_WIDTH + (column++)] = *s | currentColor;
                break;
        }
        s++;
    }
}
