#include "vga.h"
#include "stdint.h"

uint16_t column = 0;
uint16_t line = 0;
uint16_t* const vga = (uint16_t* const) 0xC00B8000;
const uint16_t defaultColor = (COLOR8_WHITE << 8)|(COLOR8_BLACK << 12);
uint16_t currentColor = defaultColor;

void SetScreenColor(uint8_t color) {
    uint8_t* video_memory = (uint8_t*)0xC00B8000;

    for (int i = 0; i < width * height * 2; i += 2) {
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

void Reset(){
    line = 0;
    column = 0;
    currentColor = defaultColor;

    for (uint16_t y = 0; y < height; y++){
        for (uint16_t x = 0; x < width; x++){
            vga[y * width + x] = ' ' | defaultColor;
        }
    }
}

void newLine(){
    if (line < height - 1){
        line++;
        column = 0;
    }
    else{
      scrollUp();
      column = 0;
    }
}

void scrollUp(){
    for (uint16_t y = 0; y < height; y++){
        for (uint16_t x = 0; x < width; x++){
            vga[(y-1) * width + x] = vga[y*width+x];
        }
    }

    for (uint16_t x = 0; x < width; x++){
        vga[(height-1) * width + x] = ' ' | currentColor;
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
                    vga[line * width + (++column)] = ' ' | currentColor;
                    break;
                }
                if (column == 0 && line != 0){
                    line--;
                    column = width;
                }
                vga[line * width + (--column)] = ' ' | currentColor;
                break;
            case '\t':
                if (column == width){
                    newLine();
                }
                uint16_t tabLen = 4 - (column % 4);
                while (tabLen != 0){
                    vga[line * width + (column++)] = ' ' | currentColor;
                    tabLen--;
                }
                break;
            default:
                if (column == width){
                    newLine();
                }

                vga[line * width + (column++)] = *s | currentColor;
                break;
        }
        s++;
    }
}
