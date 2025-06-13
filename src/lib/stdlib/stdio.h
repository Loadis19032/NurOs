#include "stdint.h"

void putc(char c);
void puts(const char* str);
void printf(const char* fmt, ...);
int * printf_number(int* argp, int length, bool sign, int radix);

#define PRINTF_STATE_START 0
#define PRINTF_STATE_FLAGS 1
#define PRINTF_STATE_WIDTH 2
#define PRINTF_STATE_PRECISION 3
#define PRINTF_STATE_LENGTH 4
#define PRINTF_STATE_SHORT 5
#define PRINTF_STATE_LONG 6
#define PRINTF_STATE_SPEC 7

#define PRINTF_LENGTH_START 0
#define PRINTF_LENGTH_SHORT 1
#define PRINTF_LENGTH_SHORT_SHORT 2
#define PRINTF_LENGTH_LONG 3
#define PRINTF_LENGTH_LONG_LONG 4