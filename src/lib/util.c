
#include "util.h"

int memcmp(const void *ptr1, const void *ptr2, size_t n) {
    const unsigned char *p1 = ptr1;
    const unsigned char *p2 = ptr2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] < p2[i]) {
            return -1;  // Первый байт меньше
        }
        if (p1[i] > p2[i]) {
            return 1;   // Первый байт больше
        }
    }

    return 0;
}

void memset(void *dest, char val, uint32_t count){
    char *temp = (char*) dest;
    for (; count != 0; count --){
        *temp++ = val;
    }
}

void outPortB(uint16_t Port, uint8_t Value){
    asm volatile ("outb %1, %0" : : "dN" (Port), "a" (Value));
}

char inPortB(uint16_t port){
    char rv;
    asm volatile("inb %1, %0": "=a"(rv):"dN"(port));
    return rv;
}

uint16_t inPortW(uint16_t port) {
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outPortW(uint16_t port, uint16_t val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

void outPortD(uint16_t port, uint32_t value) {
    asm volatile (
        "outl %0, %1" 
        : 
        : "a"(value), "Nd"(port)
    );
}

uint32_t inPortD(uint16_t port) {
    uint32_t result;
    asm volatile (
        "inl %1, %0" 
        : "=a"(result)
        : "Nd"(port)
    );
    return result;
}